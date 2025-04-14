#include "NetworkMQTT/mqtt_publisher.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread> // For sleep

namespace SensorHub::Components {

// --- Constructor / Destructor ---
MqttPublisher::MqttPublisher(std::string broker_address, std::string client_id)
    : broker_address_(std::move(broker_address)),
      client_id_(std::move(client_id))
      // client_ initialization moved to the body to handle potential exceptions
{
    try {
        // Create the asynchronous client object.
        client_ = std::make_unique<mqtt::async_client>(broker_address_, client_id_);

        // Set the callbacks for connection, message arrival, etc.
        // 'this' object implements the necessary virtual functions from mqtt::callback.
        client_->set_callback(*this);

        // Configure standard connection options.
        conn_opts_.set_keep_alive_interval(20); // Send ping request every 20 seconds.
        conn_opts_.set_clean_session(true);    // Start fresh, don't resume previous session.
        conn_opts_.set_automatic_reconnect(false); // Disable Paho's auto-reconnect; we handle it manually.
        // TODO: Add options for LWT (Last Will and Testament), SSL/TLS if needed.

        std::cout << "MQTT Publisher initialized for broker: " << broker_address_
                  << ", Client ID: " << client_id_ << std::endl;

    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT Error initializing client: " << exc.what() << std::endl;
        client_.reset(); // Ensure client is null if initialization failed.
        throw; // Re-throw to signal construction failure.
    }
}

MqttPublisher::~MqttPublisher() {
    disconnect(); // Attempt graceful disconnect in destructor.
    // unique_ptr will handle deletion of the client_ object.
    std::cout << "MQTT Publisher destroyed." << std::endl;
}

 // --- Move Semantics ---
MqttPublisher::MqttPublisher(MqttPublisher&& other) noexcept
    : broker_address_(std::move(other.broker_address_)),
      client_id_(std::move(other.client_id_)),
      client_(std::move(other.client_)), // Move the unique_ptr
      conn_opts_(std::move(other.conn_opts_)),
      connected_(other.connected_.load()), // Copy atomic bool state
      reconnect_attempts_(other.reconnect_attempts_)
      // Note: Mutexes and condition variables cannot be moved.
{
    // If the client was successfully moved, re-set its callback to 'this' new object.
    if (client_) {
        client_->set_callback(*this);
    }
    // Reset other's state where appropriate.
    other.connected_.store(false);
}

MqttPublisher& MqttPublisher::operator=(MqttPublisher&& other) noexcept {
     if (this != &other) {
        // Disconnect own client first if connected.
        disconnect();

        // Lock potentially needed if other state needs synchronized access during move.
        // std::lock_guard<std::mutex> lock(connection_mutex_); // Example if needed

        // Move resources from other.
        broker_address_ = std::move(other.broker_address_);
        client_id_ = std::move(other.client_id_);
        client_ = std::move(other.client_); // Move unique_ptr.
        conn_opts_ = std::move(other.conn_opts_);
        connected_.store(other.connected_.load()); // Copy atomic bool state.
        reconnect_attempts_ = other.reconnect_attempts_;

        // Re-set the callback for the moved client to 'this' new object.
        if (client_) {
            client_->set_callback(*this);
        }
         // Reset other's state.
        other.connected_.store(false);
     }
     return *this;
}

// --- Public Methods ---

bool MqttPublisher::connect(long timeout_ms) {
    if (!client_) {
         std::cerr << "MQTT Error: Client not initialized. Cannot connect." << std::endl;
         return false;
    }
    // Avoid reconnecting if already connected or connection attempt is pending.
    if (isConnected()) {
        // std::cout << "MQTT Info: Already connected." << std::endl;
        return true;
    }
     // Check if another connect attempt is already waiting for callback
     // Note: This simple check might have race conditions without external locking if connect() is called concurrently.
    if(connection_attempt_in_progress_) {
        std::cout << "MQTT Info: Connection attempt already in progress." << std::endl;
        return false; // Indicate we didn't start a *new* attempt now
    }


    std::cout << "MQTT: Attempting to connect to broker " << broker_address_ << "..." << std::endl;

    try {
         // Use a mutex and condition variable to wait for the asynchronous connect operation
         // to complete (signaled by on_success/on_failure or connected/connection_lost callbacks).
         std::unique_lock<std::mutex> lock(connection_mutex_);
         connection_attempt_in_progress_ = true;
         connection_succeeded_ = false; // Reset result flag before attempt

         // Initiate the asynchronous connection.
         // 'this' is passed as the action listener to receive on_success/on_failure.
         mqtt::token_ptr conntok = client_->connect(conn_opts_, nullptr, *this);

         // Wait for the condition variable to be notified by a callback, or until timeout.
         if (connection_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                    [this]{ return !connection_attempt_in_progress_; }))
         {
             // Condition met: Callback fired (either success or failure determined completion).
             // Return the outcome that was set by the callback.
             return connection_succeeded_;
         } else {
             // Condition not met: Timed out waiting for a callback.
             std::cerr << "MQTT Error: Connection attempt timed out after " << timeout_ms << " ms." << std::endl;
             // Reset the flag since the wait is over, even if callback never arrived.
             connection_attempt_in_progress_ = false;
             return false;
         }

    } catch (const mqtt::exception& exc) {
        // Exception occurred during the *initiation* of the connect call.
        std::cerr << "MQTT Error during connect initiation: " << exc.what() << std::endl;
         // Ensure flag is reset if exception occurs before wait_for.
         std::unique_lock<std::mutex> lock(connection_mutex_); // Lock before modifying flag
         connection_attempt_in_progress_ = false;
        return false;
    }
}

void MqttPublisher::disconnect(long timeout_ms) {
    // Only attempt disconnect if the client exists and is connected.
    if (client_ && client_->is_connected()) {
        std::cout << "MQTT: Disconnecting..." << std::endl;
        try {
            // Use disconnect options to specify a timeout for the operation.
            mqtt::disconnect_options disc_opts;
            disc_opts.set_timeout(std::chrono::milliseconds(timeout_ms));
            // Call disconnect and wait for the operation token to complete (or timeout).
            client_->disconnect(disc_opts)->wait();
            connected_.store(false); // Update status only after confirmed disconnect.
            std::cout << "MQTT: Disconnected." << std::endl;
        } catch (const mqtt::exception& exc) {
            std::cerr << "MQTT Error during disconnect: " << exc.what() << std::endl;
            // Assume we are disconnected at the client level even if there was an error.
            connected_.store(false);
        }
    } else {
         // Log quietly or skip if already disconnected or not initialized.
         // std::cout << "MQTT Info: Not connected or client not initialized; skipping disconnect." << std::endl;
    }
}


bool MqttPublisher::publish(const std::string& topic, const std::string& payload, int qos, bool retained) {
     // Check connection status before attempting to publish.
     if (!isConnected()) {
         std::cerr << "MQTT Error: Cannot publish, not connected." << std::endl;
         return false;
     }
     try {
          // Create a Paho message object.
          // Using make_message handles memory management.
         mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload);
         pubmsg->set_qos(qos);
         pubmsg->set_retained(retained);

         // Publish the message asynchronously.
         // For QoS 1 & 2, the delivery_complete callback will be invoked later.
         client_->publish(pubmsg);

         // Optional: Verbose logging for publish attempts.
         // std::cout << "MQTT: Published msg (QoS " << qos << ") to topic '" << topic << "'" << std::endl;
         return true; // Indicate the publish request was sent to the library.
     } catch (const mqtt::exception& exc) {
         // Exception occurred during the publish call itself.
         std::cerr << "MQTT Error publishing to topic '" << topic << "': " << exc.what() << std::endl;
         return false;
     }
}

bool MqttPublisher::isConnected() const {
    // Return the thread-safe atomic flag, updated by callbacks.
    return connected_.load();
}

// --- Private Callback Implementations ---

// Action listener callback: Invoked if the connect *action* itself fails
// (e.g., network unreachable before even trying to connect to broker).
void MqttPublisher::on_failure(const mqtt::token& tok) {
    std::cerr << "MQTT Error: Connection attempt failed (token: "
              << (tok ? tok.get_message_id() : -1) << ")" << std::endl;
    {
         // Lock mutex before modifying shared state used by connect() wait.
         std::lock_guard<std::mutex> lock(connection_mutex_);
         connection_succeeded_ = false;          // Mark outcome as failure.
         connection_attempt_in_progress_ = false; // Mark attempt as finished.
    }
    connection_cv_.notify_all(); // Notify the waiting connect() method.
    connected_.store(false);     // Ensure connection status is false.

    // Initiate reconnection logic after a failure.
    reconnect_attempts_++;
    attempt_reconnect();
}

// Action listener callback: Invoked if the connect *action* was successfully
// queued or sent by the Paho library. This *doesn't* mean connection to the
// broker is complete yet (that's handled by the 'connected' callback).
void MqttPublisher::on_success(const mqtt::token& tok) {
    // This callback isn't strictly necessary for determining connection status,
    // as the 'connected' callback is the definitive one. We primarily use it
    // to signal the waiting connect() method.
    // std::cout << "MQTT Info: Connection request sent successfully (token: "
    //           << (tok ? tok.get_message_id() : -1) << "). Waiting for broker acknowledgment." << std::endl;
     {
         // Lock mutex before modifying shared state.
         std::lock_guard<std::mutex> lock(connection_mutex_);
         // Mark potential success. The 'connected' callback will confirm.
         // We keep connection_attempt_in_progress_ = true until 'connected' or 'connection_lost'.
         connection_succeeded_ = true;
     }
     // Do NOT notify connection_cv_ here; wait for 'connected' callback.
}

// General callback: Invoked when the client successfully establishes a connection
// with the MQTT broker (broker sent CONNACK).
void MqttPublisher::connected(const std::string& cause) {
    std::cout << "MQTT: Connection successful!"
              << (cause.empty() ? "" : " Cause: " + cause) << std::endl;
    connected_.store(true);      // Set connected status to true.
    reconnect_attempts_ = 0;     // Reset reconnect counter on successful connection.
    {
        // Lock mutex before modifying shared state used by connect() wait.
        std::lock_guard<std::mutex> lock(connection_mutex_);
        connection_succeeded_ = true;          // Confirm success.
        connection_attempt_in_progress_ = false; // Mark attempt as finished.
    }
    connection_cv_.notify_all(); // Notify the waiting connect() method.
}

// General callback: Invoked when the connection to the broker is lost unexpectedly.
void MqttPublisher::connection_lost(const std::string& cause) {
    std::cerr << "MQTT Error: Connection lost."
              << (cause.empty() ? "" : " Cause: " + cause) << std::endl;
    connected_.store(false);     // Set connected status to false.
     {
         // Lock mutex before modifying shared state.
         std::lock_guard<std::mutex> lock(connection_mutex_);
         connection_attempt_in_progress_ = false; // Mark attempt as finished (if one was pending).
         connection_succeeded_ = false;          // Ensure outcome is marked as failure.
     }
     connection_cv_.notify_all(); // Notify connect() if it was waiting.

    // Initiate reconnection logic.
    reconnect_attempts_++;
    attempt_reconnect();
}

// General callback: Invoked when a message arrives on a subscribed topic.
// This example is primarily a publisher, so this is usually empty or logs unexpected messages.
void MqttPublisher::message_arrived(mqtt::const_message_ptr msg) {
    // Log if unexpected messages arrive.
    // std::cout << "MQTT Info: Unexpected message arrived on topic '" << msg->get_topic()
    //           << "': " << msg->to_string() << std::endl;
}

// General callback: Invoked when the delivery of a QoS 1 or QoS 2 message is confirmed.
void MqttPublisher::delivery_complete(mqtt::delivery_token_ptr tok) {
    // Optional: Log delivery confirmation for reliable messaging.
    // if (tok) {
    //     std::cout << "MQTT Info: Delivery complete for message token: "
    //               << tok->get_message_id() << std::endl;
    // }
}

// --- Private Reconnection Logic ---
 void MqttPublisher::attempt_reconnect() {
     // Check if maximum attempts have been reached.
     if (reconnect_attempts_ <= MAX_RECONNECT_ATTEMPTS) {
         std::cerr << "MQTT: Attempting reconnect (" << reconnect_attempts_ << "/" << MAX_RECONNECT_ATTEMPTS
                   << ") in " << RECONNECT_DELAY.count() << " seconds..." << std::endl;
         // Wait for the specified delay before trying again.
         std::this_thread::sleep_for(RECONNECT_DELAY);
         // Call connect() again to retry.
         connect();
     } else {
         std::cerr << "MQTT Error: Maximum reconnect attempts (" << MAX_RECONNECT_ATTEMPTS
                   << ") reached. Stopping reconnection attempts." << std::endl;
         // Application might need to handle this persistent failure state.
     }
 }

} // namespace SensorHub::Components