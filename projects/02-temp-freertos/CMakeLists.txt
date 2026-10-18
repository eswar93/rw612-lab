# Freestanding NXP MCUXpresso SDK application (lives in this repo, not inside the SDK).
# Needs the SDK exported once:  west mcuxsdk-export   (see docs/setup-windows-nxp.md)
cmake_minimum_required(VERSION 3.30.0)

find_package(McuxSDK 25.12.00 EXACT REQUIRED)

project(temp_freertos LANGUAGES C CXX ASM)

mcux_add_source(
    SOURCES
        temp_freertos.c
        hardware_init.c
        app.h
)

mcux_add_include(
    INCLUDES
        .
)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
