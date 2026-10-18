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

# The boot ROM reads a flash configuration block from the first 4 KB of flash (0x08000000).
# NXP's repository examples compile this board file into the application; a freestanding
# project must do it itself. It has to go into the executable target (native target_sources
# on "app"), NOT into the McuxSDK static library: nothing in our code references the table,
# so the linker would silently drop it from a library. Without it the image never boots.
target_sources(app PRIVATE
    ${SdkRootDirPath}/examples/_boards/${board}/flash_config/flash_config.c
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
