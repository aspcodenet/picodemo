cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
set(PICO_BOARD pico_w)
pico_sdk_init()


add_executable(blink
        main.c
        )

target_include_directories(blink PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)        

target_compile_definitions(blink PRIVATE
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
    CYW43_HOST_NAME="PicoW-Lcd-Node"
)        

# pull in common dependencies
target_link_libraries(blink pico_stdlib hardware_i2c pico_cyw43_arch_lwip_threadsafe_background pico_lwip_http)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)

pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)

