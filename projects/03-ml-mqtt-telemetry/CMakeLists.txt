cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ml_mqtt_telemetry)

target_sources(app PRIVATE
    src/main.cpp
    src/temp_sensor.cpp
    src/wifi_link.cpp
    src/mqtt_publisher.cpp
)
target_include_directories(app PRIVATE src)
