#include "mqtt_publisher.hpp"

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace telemetry {

MqttPublisher *MqttPublisher::instance_ = nullptr;

bool MqttPublisher::connect(const char *broker_ipv4, uint16_t port, const char *client_id, int timeout_ms)
{
    instance_ = this;
    connected_ = false;

    mqtt_client_init(&client_);

    auto *addr = reinterpret_cast<sockaddr_in *>(&broker_);
    memset(&broker_, 0, sizeof(broker_));
    addr->sin_family = AF_INET;
    addr->sin_port   = htons(port);
    if (zsock_inet_pton(AF_INET, broker_ipv4, &addr->sin_addr) != 1) {
        printk("Bad broker address: %s\n", broker_ipv4);
        return false;
    }

    client_.broker           = &broker_;
    client_.evt_cb           = &MqttPublisher::on_event;
    client_.client_id.utf8   = reinterpret_cast<const uint8_t *>(client_id);
    client_.client_id.size   = static_cast<uint32_t>(strlen(client_id));
    client_.password         = nullptr;
    client_.user_name        = nullptr;
    client_.protocol_version = MQTT_VERSION_3_1_1;
    client_.rx_buf           = rx_buffer_;
    client_.rx_buf_size      = sizeof(rx_buffer_);
    client_.tx_buf           = tx_buffer_;
    client_.tx_buf_size      = sizeof(tx_buffer_);
    client_.transport.type   = MQTT_TRANSPORT_NON_SECURE;

    const int rc = mqtt_connect(&client_);
    if (rc != 0) {
        printk("mqtt_connect failed: %d\n", rc);
        return false;
    }

    /* Wait for the broker's CONNACK */
    const int64_t deadline = k_uptime_get() + timeout_ms;
    while (!connected_ && k_uptime_get() < deadline) {
        zsock_pollfd fds{};
        fds.fd     = client_.transport.tcp.sock;
        fds.events = ZSOCK_POLLIN;
        if (zsock_poll(&fds, 1, 500) > 0 && (fds.revents & ZSOCK_POLLIN)) {
            (void)mqtt_input(&client_);
        }
    }

    if (!connected_) {
        mqtt_abort(&client_);
        return false;
    }
    return true;
}

bool MqttPublisher::publish(const char *topic, const char *payload, size_t length)
{
    if (!connected_) {
        return false;
    }

    mqtt_publish_param param{};
    param.message.topic.qos        = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 = reinterpret_cast<const uint8_t *>(topic);
    param.message.topic.topic.size = static_cast<uint32_t>(strlen(topic));
    param.message.payload.data     = reinterpret_cast<uint8_t *>(const_cast<char *>(payload));
    param.message.payload.len      = static_cast<uint32_t>(length);
    param.message_id               = next_message_id_++;
    if (next_message_id_ == 0) {
        next_message_id_ = 1;
    }
    param.dup_flag    = 0U;
    param.retain_flag = 0U;

    const int rc = mqtt_publish(&client_, &param);
    if (rc != 0) {
        printk("mqtt_publish failed: %d\n", rc);
        disconnect();
        return false;
    }
    return true;
}

void MqttPublisher::service()
{
    if (!connected_) {
        return;
    }

    zsock_pollfd fds{};
    fds.fd     = client_.transport.tcp.sock;
    fds.events = ZSOCK_POLLIN;
    if (zsock_poll(&fds, 1, 0) > 0) {
        if (fds.revents & (ZSOCK_POLLERR | ZSOCK_POLLHUP | ZSOCK_POLLNVAL)) {
            printk("MQTT socket closed\n");
            disconnect();
            return;
        }
        if (fds.revents & ZSOCK_POLLIN) {
            const int rc = mqtt_input(&client_);
            if (rc != 0 && rc != -EAGAIN) {
                printk("mqtt_input failed: %d\n", rc);
                disconnect();
                return;
            }
        }
    }

    (void)mqtt_live(&client_); /* sends a ping when the keep-alive time is nearly up */
}

void MqttPublisher::disconnect()
{
    if (connected_) {
        mqtt_abort(&client_);
    }
    connected_ = false;
}

void MqttPublisher::on_event(mqtt_client *client, const mqtt_evt *evt)
{
    ARG_UNUSED(client);

    MqttPublisher *self = instance_;
    if (self == nullptr) {
        return;
    }

    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->result == 0) {
            self->connected_ = true;
        } else {
            printk("MQTT broker refused the connection, result %d\n", evt->result);
        }
        break;
    case MQTT_EVT_DISCONNECT:
        self->connected_ = false;
        break;
    default:
        break;
    }
}

} // namespace telemetry
