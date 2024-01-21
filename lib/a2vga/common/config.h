#pragma once

#include <stdint.h>
#include <pico/stdlib.h>
#include "common/cfgtoken.h"

#ifndef CONFIG_SYSCLOCK
#warning Defaulting to 126MHz
#define CONFIG_SYSCLOCK 126.0 /* MHz */
#endif

// Pin configuration
#ifdef ANALOG_GS
#define CONFIG_PIN_APPLEBUS_DATA_BASE 16 /* 8+2 pins */
#define CONFIG_PIN_APPLEBUS_DEVSEL (CONFIG_PIN_APPLEBUS_DATA_BASE+8)
#define CONFIG_PIN_APPLEBUS_RW     (CONFIG_PIN_APPLEBUS_DATA_BASE+9)
#define CONFIG_PIN_APPLEBUS_CONTROL_BASE 26 /* 4 pins */
#define CONFIG_PIN_APPLEBUS_PHI0 14
#else
/* GPIO0-7: 6502 bus D0-D7
 * GPIO8  : DEVSEL
 * GPIO9  : R/W
 * GPIO10 : data bus transceiver direction
 * GPIO11 : data bus transceiver enable
 * GPIO12 : address bus A0-A7 transceiver enable
 * GPIO13 : address bus A8-A15 transceiver enable
 * GPIO26 : PHI0
 */
#define CONFIG_PIN_APPLEBUS_DATA_BASE 0 /* 8+2 pins */
#define CONFIG_PIN_APPLEBUS_DEVSEL (CONFIG_PIN_APPLEBUS_DATA_BASE+8)
#define CONFIG_PIN_APPLEBUS_RW     (CONFIG_PIN_APPLEBUS_DATA_BASE+9)
#define CONFIG_PIN_APPLEBUS_CONTROL_BASE 10 /* 4 pins */
#define CONFIG_PIN_APPLEBUS_PHI0 26
#endif // !ANALOG_GS

#define CONFIG_ABUS_PIO pio1

#ifdef FUNCTION_VGA
#ifdef ANALOG_GS
#define CONFIG_PIN_HSYNC 13
#define CONFIG_PIN_VSYNC 12
#define CONFIG_PIN_RGB_BASE 0 /* 12 pins */
#else
#define CONFIG_PIN_HSYNC 28
#define CONFIG_PIN_VSYNC 27
#define CONFIG_PIN_RGB_BASE 14 /* 9 pins */
#endif // !ANALOG_GS

// Other resources
#define CONFIG_VGA_PIO pio0
#define CONFIG_VGA_SPINLOCK_ID 31

extern volatile uint32_t mono_palette;
extern volatile uint8_t terminal_tbcolor;
extern volatile uint8_t terminal_border;
#endif // FUNCTION_VGA

#ifdef FUNCTION_USB
#define CONFIG_PIN_IRQ   27 /* same as VGA VSYNC */
#endif

#ifdef FUNCTION_Z80
typedef enum {
    SERIAL_LOOP = 0,
    SERIAL_UART,
    SERIAL_USB,
    SERIAL_WIFI,
    SERIAL_PRINTER,
} serialmux_t;

extern volatile serialmux_t serialmux[2];

typedef enum {
    USB_HOST_CDC,
    USB_GUEST_CDC,
    USB_GUEST_MIDI,
} usbmux_t;

extern volatile usbmux_t usbmux;

typedef enum {
    WIFI_CLIENT = 0,
    WIFI_AP,
} wifimode_t;

extern volatile wifimode_t wifimode;

extern uint8_t wifi_ssid[32];
extern uint8_t wifi_psk[32];

extern uint32_t wifi_address;
extern uint32_t wifi_netmask;

extern uint8_t jd_host[32];
extern uint16_t jd_port;

#endif

typedef enum {
    MACHINE_II = 0,
    MACHINE_IIE = 1,
    MACHINE_IIGS = 2,
    MACHINE_PRAVETZ = 6,
    MACHINE_AGAT7 = 7,
    MACHINE_BASIS = 8,
    MACHINE_AGAT9 = 9,
    MACHINE_INVALID = 0xfe,
    MACHINE_AUTO = 0xff
} compat_t;

extern volatile compat_t cfg_machine;
extern volatile compat_t current_machine;


void default_config();
int make_config(uint32_t rev);
bool read_config(bool onetime);
bool write_config(bool onetime);

void config_handler();

void dmacopy32(uint32_t *start, uint32_t *end, uint32_t *source);

#if 1
#define DELAYED_COPY_CODE(n) __noinline __attribute__((section(".delayed_code."))) n
#else
#define DELAYED_COPY_CODE(n) __noinline __time_critical_func(n)
#endif

#if 1
#define DELAYED_COPY_DATA(n) __attribute__((section(".delayed_data."))) n
#else
#define DELAYED_COPY_DATA(n) n
#endif
