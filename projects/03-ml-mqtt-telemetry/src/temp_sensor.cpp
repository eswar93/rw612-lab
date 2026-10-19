#include "temp_sensor.hpp"

#include <zephyr/drivers/sensor.h>

namespace telemetry {

Reading TempSensor::read() const
{
    if (sensor_sample_fetch(dev_) != 0) {
        return {Error::Fetch, 0};
    }

    sensor_value value{};
    if (sensor_channel_get(dev_, SENSOR_CHAN_AMBIENT_TEMP, &value) != 0) {
        return {Error::Channel, 0};
    }

    return {Error::None, static_cast<int32_t>(sensor_value_to_milli(&value))};
}

} // namespace telemetry
