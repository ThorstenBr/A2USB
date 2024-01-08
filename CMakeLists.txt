cmake_minimum_required(VERSION 3.17)

set(PICO_STDIO_UART OFF)
set(PICO_STDIO_USB  OFF)

if (NOT PICO_SDK_PATH)
   message(FATAL_ERROR "PICO_SDK_PATH is not set! Provide PICO_SDK_PATH as an environment variable.")
else()
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# firmware version
set(FW_VERSION "1.3")

set(PROJECT A2USB)
set(BINARY_NAME "A2USB")

project(${PROJECT} C CXX ASM)

set(FAMILY rp2040)
set(BOARD pico_sdk)

enable_language(C CXX ASM)
pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_LED=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_USB=1 ")

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "MOUSE-")
  message(STATUS "MOUSE support is enabled...")
  set(BINARY_NAME "${BINARY_NAME}-MOUSE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_MOUSE=1 ")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-PAL")
  message(STATUS "Selected PAL/50Hz default...")
  set(BINARY_NAME "${BINARY_NAME}-PAL")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_PAL=1 ")
else()
  message(STATUS "Selected PAL/50Hz default...")
  set(BINARY_NAME "${BINARY_NAME}-NTSC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_NTSC=1 ")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-DEBUG")
  message(WARNING "Building DEBUG version for bench testing! *****************************************************")
  set(BINARY_NAME "${BINARY_NAME}-DEBUG")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_OUTPUT=1")
  set(DEBUG_MODE 1)
  set(PICO_STDIO_UART ON)
else()
  set(DEBUG_MODE 0)
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-PROFILER")
  message(WARNING "Building PROFILER version! *****************************************************")
  set(BINARY_NAME "${BINARY_NAME}-PROFILER")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_PROFILER=1")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-LOGGER")
  message(WARNING "Building LOGGER firmware for DEBUGGING! *****************************************************")
  set(BINARY_NAME "${BINARY_NAME}-LOGGER")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_LOGGING=1")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-A2VGA")
  message(STATUS "Building for A2VGA platform...")
  set(BINARY_NAME "${BINARY_NAME}-A2VGA")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLATFORM_A2VGA=1 ")
  set(A2_PLATFORM "A2VGA")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-4ns")
    set(BINARY_NAME "${BINARY_NAME}-4ns")
    message(STATUS "SYSCLOCK is 252MHz")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_SYSCLOCK=252 -DPICO_FLASH_SPI_CLKDIV=8 -DOVERCLOCKED=1")
elseif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-8ns")
    set(BINARY_NAME "${BINARY_NAME}-8ns")
    message(STATUS "SYSCLOCK is 126MHz")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_SYSCLOCK=126")
else()
    message(FATAL_ERROR "You must specify -4ns (252MHz) or -8ns (126MHz) speed.")
endif()

message(STATUS "Building firmware version ${FW_VERSION}")
set(BINARY_NAME "${BINARY_NAME}-v${FW_VERSION}")

# executable name
add_executable(${BINARY_NAME})

if(${DEBUG_MODE} MATCHES 1)
    target_link_libraries(${BINARY_NAME} PUBLIC pico_stdio_uart)
endif()

if (${A2_PLATFORM} MATCHES "A2VGA")
    include("lib/a2vga/a2vga.cmake")
    a2vgaConfig(${BINARY_NAME})
endif()

# enable compiler warnings
add_compile_options(-Wall ${A2_OPTIONS})

# disable std output via USB
pico_enable_stdio_usb(${BINARY_NAME} 0)

# disable std output via serial UART pins
pico_enable_stdio_uart(${BINARY_NAME} 0)

# source list
target_sources(${BINARY_NAME} PUBLIC
        lib/dma/dmacopy.c
        source/main.c
        source/usb/hid_app.c
        source/usb/usb.c
#        source/usb/businterface.c # module is inlined instead
        source/mouse/MouseInterfaceCard.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${BINARY_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/source
        ${CMAKE_CURRENT_LIST_DIR}/source/usb)

# configure outputs target
pico_add_extra_outputs(${BINARY_NAME})

# set libraries to be linked
target_link_libraries(${BINARY_NAME} PUBLIC
        pico_multicore
        pico_stdlib
        hardware_dma
        tinyusb_host
#        tinyusb_additions
        )

pico_set_linker_script(${BINARY_NAME} ${PROJECT_SOURCE_DIR}/build/delayed_copy.ld)

pico_add_extra_outputs(${BINARY_NAME})
