/*
 * Project 3, step M1: read the on-board P3T1755 through Zephyr's sensor API, in C++17.
 * Later steps add Wi-Fi, MQTT, and the ML pipeline (see README).
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "temp_sensor.hpp"

namespace {
constexpr int kSamplePeriodMs = 1000;
}

int main(void)
{
    /* The sensor object is created inside main(), not as a global: Zephyr runs global C++
     * constructors before main(), and I want to control when hardware is touched. */
    telemetry::TempSensor sensor(DEVICE_DT_GET(DT_ALIAS(ambient_temp0)));

    if (!sensor.ready()) {
        printk("Temperature sensor is not ready (check the overlay and the I2C pins)\n");
        return 0;
    }

    printk("Project 3 (C++17): reading the P3T1755 every %d ms\n", kSamplePeriodMs);

    for (unsigned int count = 0;; ++count) {
        const telemetry::Reading r = sensor.read();

        if (r.error != telemetry::Error::None) {
            printk("#%u  sensor error %d\n", count, static_cast<int>(r.error));
        } else {
            const int32_t m          = r.milli_celsius;
            const char *sign         = (m < 0) ? "-" : "";
            const uint32_t magnitude = static_cast<uint32_t>((m < 0) ? -m : m);
            printk("#%u  Temperature: %s%u.%03u C\n", count, sign, static_cast<unsigned int>(magnitude / 1000U),
                   static_cast<unsigned int>(magnitude % 1000U));
        }

        k_msleep(kSamplePeriodMs);
    }

    return 0;
}
