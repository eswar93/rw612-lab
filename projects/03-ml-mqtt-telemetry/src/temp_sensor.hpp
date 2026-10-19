#pragma once

#include <cstdint>

#include <zephyr/device.h>

namespace telemetry {

enum class Error : int32_t {
    None = 0,
    Fetch,   /* the driver could not read the sensor */
    Channel, /* the driver could not give us the temperature value */
};

struct Reading {
    Error error;
    int32_t milli_celsius; /* thousandths of a degree C; only valid when error == None */
};

/* A thin wrapper around Zephyr's sensor API. No heap, no exceptions. */
class TempSensor {
public:
    explicit TempSensor(const device *dev) : dev_(dev) {}

    bool ready() const { return device_is_ready(dev_); }
    Reading read() const;

private:
    const device *dev_;
};

} // namespace telemetry
