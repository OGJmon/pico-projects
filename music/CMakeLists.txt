cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(music C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

#include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
	add_executable(music
   	     	 music.c
		 glcdfont.c
		 Adafruit_ST7735.c
		 graphics.c
		 bootrom_api.c
		 syscalls.c
		 tft_stdout.c
   	     )
	
	# pull in common dependencies
	target_link_libraries(
		music
		pico_stdlib
		hardware_spi
		hardware_gpio
		hardware_pwm
		pico_cyw43_arch_none
	)
	pico_enable_stdio_usb(music 1)
	pico_enable_stdio_uart(music 0)
	
	# create map/bin/hex file etc.
	pico_add_extra_outputs(music)
	
	# add url via pico_set_program_url
#	example_auto_set_url(keypad)
elseif(PICO_ON_DEVICE)
	message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
