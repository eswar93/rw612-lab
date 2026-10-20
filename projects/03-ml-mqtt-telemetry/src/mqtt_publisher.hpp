#pragma once

#include <stddef.h>
#include <stdint.h>

#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>

namespace telemetry {

/* A small wrapper around Zephyr's MQTT client: connect, publish (QoS 0), keep the link alive.
 * Plain MQTT 3.1.1 over TCP, no TLS yet. Only one MqttPublisher may exist (see instance_). */
class MqttPublisher {
public:
    bool connect(const char *broker_ipv4, uint16_t port, const char *client_id, int timeout_ms);
    bool publish(const char *topic, const char *payload, size_t length);
    void service();
    void disconnect();
    bool connected() const { return connected_; }

private:
    static void on_event(mqtt_client *client, const mqtt_evt *evt);

    static MqttPublisher *instance_;

    mqtt_client client_{};
    sockaddr_storage broker_{};
    uint8_t rx_buffer_[256] = {};
    uint8_t tx_buffer_[512] = {};
    volatile bool connected_ = false;
    uint16_t next_message_id_ = 1;
};

} // namespace telemetry
