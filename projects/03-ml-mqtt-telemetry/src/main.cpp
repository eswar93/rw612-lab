/*
 * Project 3, step M2: read the P3T1755 once a second and publish the reading over MQTT.
 * If no Wi-Fi/broker settings are compiled in (secrets.conf), it only prints locally, like M1.
 */
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "mqtt_publisher.hpp"
#include "temp_sensor.hpp"
#include "wifi_link.hpp"

namespace {

constexpr int kSamplePeriodMs  = 1000;
constexpr int kWifiTimeoutMs   = 30000;
constexpr int kMqttTimeoutMs   = 5000;
constexpr int kRetryDelayMs    = 5000;

bool network_configured()
{
    return CONFIG_APP_WIFI_SSID[0] != '\0' && CONFIG_APP_MQTT_BROKER_ADDR[0] != '\0';
}

/* Bring Wi-Fi and the MQTT session up if they are not. Returns true when ready to publish. */
bool ensure_network(telemetry::WifiLink &wifi, telemetry::MqttPublisher &mqtt)
{
    if (!wifi.is_up()) {
        mqtt.disconnect();
        printk("Connecting to Wi-Fi \"%s\" ...\n", CONFIG_APP_WIFI_SSID);
        const telemetry::WifiCredentials credentials{CONFIG_APP_WIFI_SSID, CONFIG_APP_WIFI_PSK,
                                                     CONFIG_APP_WIFI_SECURITY};
        if (!wifi.connect(credentials, kWifiTimeoutMs)) {
            return false;
        }
        printk("Wi-Fi is up\n");
    }

    if (!mqtt.connected()) {
        printk("Connecting to MQTT broker %s:%d ...\n", CONFIG_APP_MQTT_BROKER_ADDR, CONFIG_APP_MQTT_BROKER_PORT);
        if (!mqtt.connect(CONFIG_APP_MQTT_BROKER_ADDR, CONFIG_APP_MQTT_BROKER_PORT, CONFIG_APP_DEVICE_ID,
                          kMqttTimeoutMs)) {
            printk("MQTT connect failed\n");
            return false;
        }
        printk("MQTT connected\n");
    }
    return true;
}

} // namespace

int main(void)
{
    /* Objects are created inside main(), not as globals: Zephyr runs global C++ constructors
     * before main(), and I want to control when hardware and the network are touched. */
    telemetry::TempSensor sensor(DEVICE_DT_GET(DT_ALIAS(ambient_temp0)));
    telemetry::WifiLink wifi;
    telemetry::MqttPublisher mqtt;

    if (!sensor.ready()) {
        printk("Temperature sensor is not ready (check the overlay and the I2C pins)\n");
        return 0;
    }

    const bool use_network = network_configured() && wifi.start();
    if (use_network) {
        printk("Project 3: publishing to %s:%d, topic %s\n", CONFIG_APP_MQTT_BROKER_ADDR,
               CONFIG_APP_MQTT_BROKER_PORT, CONFIG_APP_MQTT_TOPIC);
    } else {
        printk("Project 3: no Wi-Fi/broker settings (see secrets.conf.example), printing locally only\n");
    }

    for (uint32_t seq = 0;; ++seq) {
        bool ready_to_publish = false;
        if (use_network) {
            ready_to_publish = ensure_network(wifi, mqtt);
            if (!ready_to_publish) {
                k_msleep(kRetryDelayMs);
            }
        }

        const telemetry::Reading r = sensor.read();
        if (r.error != telemetry::Error::None) {
            printk("#%u  sensor error %d\n", static_cast<unsigned int>(seq), static_cast<int>(r.error));
        } else {
            const int32_t m          = r.milli_celsius;
            const char *sign         = (m < 0) ? "-" : "";
            const uint32_t magnitude = static_cast<uint32_t>((m < 0) ? -m : m);
            printk("#%u  Temperature: %s%u.%03u C\n", static_cast<unsigned int>(seq), sign,
                   static_cast<unsigned int>(magnitude / 1000U), static_cast<unsigned int>(magnitude % 1000U));

            if (ready_to_publish) {
                /* Whole numbers only: millidegrees keep the message exact and need no float printing */
                char message[160];
                const int length = snprintf(message, sizeof(message),
                                            "{\"device\":\"%s\",\"seq\":%u,\"uptime_ms\":%u,\"temp_mC\":%d}",
                                            CONFIG_APP_DEVICE_ID, static_cast<unsigned int>(seq),
                                            static_cast<unsigned int>(k_uptime_get_32()), static_cast<int>(m));
                if (length > 0 && length < static_cast<int>(sizeof(message))) {
                    (void)mqtt.publish(CONFIG_APP_MQTT_TOPIC, message, static_cast<size_t>(length));
                }
            }
        }

        if (use_network) {
            mqtt.service();
        }
        k_msleep(kSamplePeriodMs);
    }

    return 0;
}
