#include "LinuxI2C_Manager/linux_i2c_manager.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <unistd.h>     // For open, close, read, write
#include <fcntl.h>      // For O_RDWR
#include <sys/ioctl.h>  // For ioctl
#include <linux/i2c-dev.h>// For I2C_SLAVE, I2C_SMBUS_* constants
#include <linux/i2c.h>  // For struct i2c_msg, struct i2c_rdwr_ioctl_data
#include <cerrno>       // For errno
#include <cstring>      // For strerror
#include <iomanip>      // For std::hex, std::dec

namespace SensorHub::Components {

// --- Constructor / Destructor ---
I2C_Manager::I2C_Manager(std::string bus_device_path)
    : bus_path_(std::move(bus_device_path)) {
    // Lock immediately during construction phase
    std::lock_guard<std::mutex> lock(bus_mutex_);

    fd_ = open(bus_path_.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw std::runtime_error("I2C_Manager: Failed to open bus " + bus_path_ + ": " + strerror(errno));
    }
    std::cout << "I2C_Manager: Opened bus " << bus_path_ << std::endl;
}

I2C_Manager::~I2C_Manager() {
    // Lock before closing
    std::lock_guard<std::mutex> lock(bus_mutex_);

    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
        std::cout << "I2C_Manager: Closed bus " << bus_path_ << std::endl;
    }
}

// --- Move Semantics ---
I2C_Manager::I2C_Manager(I2C_Manager&& other) noexcept
    : bus_path_(std::move(other.bus_path_)),
      fd_(other.fd_),
      current_address_(other.current_address_)
// Note: Mutex is not moved, the new object gets its own default-constructed mutex.
{
    // Prevent the moved-from object's destructor from closing the fd
    other.fd_ = -1;
}

I2C_Manager& I2C_Manager::operator=(I2C_Manager&& other) noexcept {
     if (this != &other) {
        // Lock both mutexes safely to prevent deadlock (using std::lock)
         std::lock(bus_mutex_, other.bus_mutex_);
         std::lock_guard<std::mutex> self_lock(bus_mutex_, std::adopt_lock);
         std::lock_guard<std::mutex> other_lock(other.bus_mutex_, std::adopt_lock);

        // Close the current file descriptor if it's open
        if (fd_ >= 0) {
             close(fd_);
        }

        // Move resources from other
        bus_path_ = std::move(other.bus_path_);
        fd_ = other.fd_;
        current_address_ = other.current_address_;

        // Reset the moved-from object
        other.fd_ = -1;
        other.current_address_ = 0;
     }
     return *this;
}

// --- Private Helper ---
bool I2C_Manager::setActiveDevice(uint8_t device_address) {
    // Assumes bus_mutex_ is already held by the calling public method
    if (fd_ < 0) {
        std::cerr << "I2C_Manager Error: Bus not open." << std::endl;
        return false;
    }
    // Optimization: Avoid unnecessary ioctl if address hasn't changed
    if (current_address_ == device_address) {
        return true;
    }
    // Set the I2C slave address for subsequent communication
    if (ioctl(fd_, I2C_SLAVE, device_address) < 0) {
        std::cerr << "I2C_Manager Error: Failed to set slave address 0x"
                  << std::hex << static_cast<int>(device_address) << ": "
                  << strerror(errno) << std::dec << std::endl;
        current_address_ = 0; // Invalidate cache on error
        return false;
    }
    current_address_ = device_address; // Cache the new address
    return true;
}

// --- Public I2C Operations ---

bool I2C_Manager::writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) {
    std::lock_guard<std::mutex> lock(bus_mutex_); // Lock the bus for this transaction

    if (!setActiveDevice(device_address)) {
        return false;
    }

    // Prepare buffer: [register_address, value]
    uint8_t buffer[2] = {reg, value};
    // Write the buffer (register address followed by data byte)
    if (write(fd_, buffer, 2) != 2) {
        std::cerr << "I2C_Manager Error: Failed writeByteData to addr 0x"
                  << std::hex << static_cast<int>(device_address) << " reg 0x" << static_cast<int>(reg)
                  << ": " << strerror(errno) << std::dec << std::endl;
        return false;
    }
    return true;
}

std::optional<uint8_t> I2C_Manager::readByteData(uint8_t device_address, uint8_t reg) {
    std::lock_guard<std::mutex> lock(bus_mutex_); // Lock the bus

    if (!setActiveDevice(device_address)) {
        return std::nullopt;
    }

    // Write the register address we want to read from
    if (write(fd_, &reg, 1) != 1) {
         std::cerr << "I2C_Manager Error: Failed write reg address 0x" << std::hex << static_cast<int>(reg)
                   << " for readByteData from addr 0x" << static_cast<int>(device_address)
                   << ": " << strerror(errno) << std::dec << std::endl;
        return std::nullopt;
    }

    // Read the data byte back
    uint8_t value = 0;
    if (read(fd_, &value, 1) != 1) {
         std::cerr << "I2C_Manager Error: Failed readByteData from addr 0x" << std::hex << static_cast<int>(device_address)
                   << " reg 0x" << static_cast<int>(reg) << ": " << strerror(errno) << std::dec << std::endl;
        return std::nullopt;
    }
    return value;
}

std::optional<std::vector<uint8_t>> I2C_Manager::readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) {
     std::lock_guard<std::mutex> lock(bus_mutex_); // Lock the bus

     if (count == 0) return std::vector<uint8_t>(); // Return empty vector if count is 0
     if (!setActiveDevice(device_address)) {
        return std::nullopt;
     }

     // Write the starting register address
     if (write(fd_, &start_reg, 1) != 1) {
         std::cerr << "I2C_Manager Error: Failed write start reg 0x" << std::hex << static_cast<int>(start_reg)
                   << " for readBlockData from addr 0x" << static_cast<int>(device_address)
                   << ": " << strerror(errno) << std::dec << std::endl;
         return std::nullopt;
     }

     // Read the block of data
     std::vector<uint8_t> buffer(count);
     ssize_t bytes_read = read(fd_, buffer.data(), count);

     // Check for read errors
     if (bytes_read < 0) {
        std::cerr << "I2C_Manager Error: Failed readBlockData (" << count << " bytes) from addr 0x"
                  << std::hex << static_cast<int>(device_address) << " reg 0x" << static_cast<int>(start_reg)
                  << ": " << strerror(errno) << std::dec << std::endl;
        return std::nullopt;
     }
     // Check for partial reads (might happen, sometimes okay, sometimes an error)
     if (static_cast<size_t>(bytes_read) != count) {
         std::cerr << "I2C_Manager Warning: Read only " << bytes_read << " of " << count
                   << " bytes from addr 0x" << std::hex << static_cast<int>(device_address)
                   << " reg 0x" << static_cast<int>(start_reg) << std::dec << std::endl;
         buffer.resize(bytes_read); // Adjust vector size to actual bytes read
     }

     return buffer; // Return the (potentially resized) vector
}

 bool I2C_Manager::writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) {
     std::lock_guard<std::mutex> lock(bus_mutex_); // Lock the bus

     if (data.empty()) return true; // Nothing to write
     if (!setActiveDevice(device_address)) {
        return false;
     }

     // Create a single buffer: [start_reg, data_byte_0, data_byte_1, ...]
     // Note: Check device datasheet & I2C/SMBus limitations on block write size.
     std::vector<uint8_t> buffer;
     buffer.reserve(1 + data.size());
     buffer.push_back(start_reg); // First byte is the register address
     buffer.insert(buffer.end(), data.begin(), data.end()); // Append the data

     ssize_t bytes_to_write = static_cast<ssize_t>(buffer.size());
     // Write the entire buffer in one go
     if (write(fd_, buffer.data(), bytes_to_write) != bytes_to_write) {
          std::cerr << "I2C_Manager Error: Failed writeBlockData (" << data.size() << " bytes) to addr 0x"
                    << std::hex << static_cast<int>(device_address) << " reg 0x" << static_cast<int>(start_reg)
                    << ": " << strerror(errno) << std::dec << std::endl;
          return false;
     }
     return true;
 }

 bool I2C_Manager::probeDevice(uint8_t device_address) {
     std::lock_guard<std::mutex> lock(bus_mutex_); // Lock the bus

     if (fd_ < 0) {
         std::cerr << "I2C_Manager Error: Bus not open for probe." << std::endl;
         return false;
     }

     // Attempt to set the slave address. A failure often indicates no device.
     if (ioctl(fd_, I2C_SLAVE, device_address) < 0) {
         // Specific errors like EIO (I/O error) or ENXIO (No such device or address)
         // typically mean no device acknowledged at this address.
         if (errno == EIO || errno == ENXIO) {
             // This is expected if no device is present, not necessarily an "error" log message.
             // Restore previous address if necessary, as ioctl might have side effects
             if (current_address_ != 0 && current_address_ != device_address) {
                 ioctl(fd_, I2C_SLAVE, current_address_); // Best effort restore
             }
             return false; // No device acknowledged
         } else {
             // Other unexpected error during ioctl
             std::cerr << "I2C_Manager Error: Failed probe ioctl for addr 0x"
                       << std::hex << static_cast<int>(device_address) << ": "
                       << strerror(errno) << std::dec << std::endl;
             current_address_ = 0; // Invalidate address cache on error
             return false;
         }
     }

     // If ioctl succeeded, a device responded. Cache the address.
     current_address_ = device_address;
     return true; // Device acknowledged
 }

 const std::string& I2C_Manager::getBusPath() const {
     return bus_path_;
 }


} // namespace SensorHub::Components