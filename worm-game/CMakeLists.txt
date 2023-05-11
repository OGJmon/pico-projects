cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(test-code C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
	add_executable(worm-game
   	     main.c
		 glcdfont.c
		 Adafruit_ST7735.c
		 graphics.c
		 keypad.c
		player.c
		field.c
		green.c
		red.c
		syscalls.c
		debug.c
		bootrom_api.c
		tft_stdout.c
		field_public.c
		rnd.c
   	     )
	
	# pull in common dependencies
	target_link_libraries(
		worm-game
		pico_stdlib
		hardware_spi
		hardware_gpio
		pico_cyw43_arch_none
	)
	pico_enable_stdio_usb(worm-game 1)
	pico_enable_stdio_uart(worm-game 0)
	
	# create map/bin/hex file etc.
	pico_add_extra_outputs(worm-game)
	
	# add url via pico_set_program_url
#	example_auto_set_url(worm-game)
elseif(PICO_ON_DEVICE)
	message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
