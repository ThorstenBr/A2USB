cmake_minimum_required(VERSION 3.13)

if (NOT PICO_SDK_PATH)
   message(FATAL_ERROR "PICO_SDK_PATH is not set! Provide PICO_SDK_PATH as an environment variable.")
else()
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# firmware version
set(FW_VERSION "1.0")

set(PROJECT A2-USB)

project(${PROJECT} C CXX ASM)

set(FAMILY rp2040)
set(BOARD pico_sdk)

include(${PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)
family_initialize_project(A2-USB ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_LED=1")

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-USB")
 message(STATUS "USB support is enabled...")
 set(BINARY_TAGS "${BINARY_TAGS}-USB")
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_USB=1 ")

 if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-MOUSE")
  message(STATUS "MOUSE support is enabled...")
  set(BINARY_TAGS "${BINARY_TAGS}-MOUSE")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_MOUSE=1 ")
 endif()
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-DEBUG")
  message(WARNING "Building DEBUG version for bench testing! *****************************************************")
  set(BINARY_TAGS "${BINARY_TAGS}-DEBUG")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_OUTPUT=1")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-LOGGER")
  message(WARNING "Building LOGGER firmware for DEBUGGING! *****************************************************")
  set(BINARY_TAGS "${BINARY_TAGS}-LOGGER")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUNCTION_LOGGING=1")
endif()

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-4ns")
    set(BINARY_TAGS "${BINARY_TAGS}-4ns")
    message(STATUS "SYSCLOCK is 252MHz")
elseif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-8ns")
    set(BINARY_TAGS "${BINARY_TAGS}-8ns")
    message(STATUS "SYSCLOCK is 126MHz")
else()
    message(FATAL_ERROR "You must specify -4ns (252MHz) or -8ns (126MHz) speed.")
endif()

message(STATUS "Building firmware version ${FW_VERSION}")
set(BINARY_TAGS "${BINARY_TAGS}-v${FW_VERSION}")

# executable name
add_executable(A2${BINARY_TAGS})

if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-4ns")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_SYSCLOCK=252 -DPICO_FLASH_SPI_CLKDIV=8 -DOVERCLOCKED=1")
 if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
     pico_generate_pio_header(A2${BINARY_TAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/common/abus-gs-4ns.pio)
 else()
    pico_generate_pio_header(A2${BINARY_TAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/common/abus-4ns.pio)
 endif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
elseif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-8ns")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_SYSCLOCK=126")
 if(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
    pico_generate_pio_header(A2${BINARY_TAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/common/abus-gs-8ns.pio)
 else()
    pico_generate_pio_header(A2${BINARY_TAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/common/abus-8ns.pio)
 endif(${CMAKE_CURRENT_BINARY_DIR} MATCHES "-gs")
endif()

# enable compiler warnings
add_compile_options(-Wall ${A2_OPTIONS})


# disable std output via USB
pico_enable_stdio_usb(A2${BINARY_TAGS} 0)

# disable std output via serial UART pins
pico_enable_stdio_uart(A2${BINARY_TAGS} 0)

# source list
target_sources(A2${BINARY_TAGS} PUBLIC
        common/main.c
        common/abus.c
#        common/config.c  # disabled for now
        common/dmacopy.c
        common/buffers.c
#        common/flash.c   # disabled for now
        usb/hid_app.c
        usb/usb.c
#        usb/businterface.c # module is inlined instead
        mouse/MouseInterfaceCard.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(A2${BINARY_TAGS} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/usb)

# configure family target
family_configure_target(A2${BINARY_TAGS})

# set libraries to be linked
target_link_libraries(A2${BINARY_TAGS} PUBLIC
        pico_multicore
        pico_stdlib
        tinyusb_host
        hardware_resets
        hardware_irq
        hardware_dma
        hardware_pio
        hardware_flash
        )

pico_set_linker_script(A2${BINARY_TAGS} ${PROJECT_SOURCE_DIR}/delayed_copy.ld)

pico_add_extra_outputs(A2${BINARY_TAGS})
