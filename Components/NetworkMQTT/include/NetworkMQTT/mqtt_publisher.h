#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory> // For std::unique_ptr
#include "mqtt/async_client.h" // Paho C++ header

namespace SensorHub::Components {

class MqttPublisher : public virtual mqtt::callback,
                      public virtual mqtt::iaction_listener {
public:
    /**
     * @brief Constructor.
     * @param broker_address The address of the MQTT broker (e.g., "tcp://localhost:1883").
     * @param client_id The unique client ID for this connection.
     * @throws mqtt::exception if client creation fails.
     */
    MqttPublisher(std::string broker_address, std::string client_id);
    /**
     * @brief Destructor. Attempts to disconnect gracefully.
     */
    ~MqttPublisher() override;

    /**
     * @brief Connects to the MQTT broker asynchronously.
     * Uses internal retry logic on failure based on MAX_RECONNECT_ATTEMPTS.
     * @param timeout_ms Maximum time to wait for the initial connection attempt.
     * @return True if connection attempt was initiated successfully, false otherwise.
     */
    bool connect(long timeout_ms = 30000);

    /**
     * @brief Disconnects from the MQTT broker asynchronously.
     * @param timeout_ms Maximum time to wait for disconnection action.
     */
    void disconnect(long timeout_ms = 10000);

    /**
     * @brief Publishes a message to a specific topic asynchronously.
     * @param topic The MQTT topic to publish to.
     * @param payload The message payload.
     * @param qos The Quality of Service level (0, 1, or 2). Default 0.
     * @param retained Whether the message should be retained by the broker. Default false.
     * @return True if the publish request was accepted by the client library, false otherwise (e.g., not connected).
     */
    bool publish(const std::string& topic, const std::string& payload, int qos = 0, bool retained = false);

    /**
     * @brief Checks if the client believes it is currently connected.
     * Reflects the state updated by connection/disconnection callbacks.
     * @return True if connected, false otherwise.
     */
    bool isConnected() const;

    // Delete copy/assignment
    MqttPublisher(const MqttPublisher&) = delete;
    MqttPublisher& operator=(const MqttPublisher&) = delete;
    // Allow move semantics
    MqttPublisher(MqttPublisher&&) noexcept;
    MqttPublisher& operator=(MqttPublisher&&) noexcept;

private:
    // --- Paho MQTT Callback overrides ---
    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;
    void connected(const std::string& cause) override;
    void connection_lost(const std::string& cause) override;
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr tok) override;

    // --- Member Variables ---
    std::string broker_address_;
    std::string client_id_;
    std::unique_ptr<mqtt::async_client> client_; // MQTT asynchronous client instance
    mqtt::connect_options conn_opts_;           // Connection options

    std::atomic<bool> connected_{false}; // Thread-safe flag for connection status

    // Synchronization for connect() waiting on callback
    std::mutex connection_mutex_;
    std::condition_variable connection_cv_;
    bool connection_attempt_in_progress_ = false; // Tracks if connect() is waiting
    bool connection_succeeded_ = false;          // Result for connect() wait

    // --- Reconnection Logic ---
    const int MAX_RECONNECT_ATTEMPTS = 5; // Max attempts before giving up (for now)
    const std::chrono::seconds RECONNECT_DELAY = std::chrono::seconds(5); // Delay between attempts
    int reconnect_attempts_ = 0; // Current attempt counter

    /**
     * @brief Internal helper to schedule and attempt reconnection after a delay.
     */
    void attempt_reconnect();
};

} // namespace SensorHub::Components