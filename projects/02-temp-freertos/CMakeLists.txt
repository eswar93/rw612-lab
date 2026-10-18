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

# NXP's mcux_convert_binary() only works for repository-style examples. With find_package(McuxSDK)
# the SDK is a library and our program is the target "app", so we make the flash files ourselves.
# west flash (J-Link runner) looks for build/app.hex or build/app.bin.
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:app> $<TARGET_FILE_DIR:app>/app.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:app> $<TARGET_FILE_DIR:app>/app.bin
    COMMENT "Creating app.hex and app.bin"
    VERBATIM
)
