/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2024 Thorsten Brehm
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifdef FUNCTION_USB

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "common/buffers.h"
#include "tusb.h"

#ifdef FUNCTION_MOUSE
  #include "mouse/MouseInterfaceCard.h"
#endif

#ifdef DEBUG_OUTPUT
  #include "hardware/uart.h"
  #include "pico/stdlib.h"
#endif

/* VARIABLES */
static bool UsbConnected = false;

/* FORWARD DECLARATIONS */
#ifdef FEATURE_CDC_SUPPORT
extern void cdc_app_task(void);
#endif

extern void hid_app_task(void);
extern void hid_app_init(void);

#define millis() (get_absolute_time()/1000)

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tuh_mount_cb(uint8_t dev_addr)
{
  // application set-up
#ifdef DEBUG_OUTPUT
  printf("A device with address %d is mounted\r\n", dev_addr);
#endif
  UsbConnected = true;
}

void tuh_umount_cb(uint8_t dev_addr)
{
  // application tear-down
#ifdef DEBUG_OUTPUT
  printf("A device with address %d is unmounted \r\n", dev_addr);
#endif
  UsbConnected = false;
}

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
#ifdef FUNCTION_LED

void usb_led_blinking(void)
{
  static uint32_t start_ms = 0;
#if 1
  // only show activity when USB mouse sends reports
  uint32_t interval_ms = 2000;
  if (millis() - start_ms < interval_ms)
    return;
  gpio_put(PICO_DEFAULT_LED_PIN, 0);
#else
  // continuous blinking
  static bool led_state = false;
  uint32_t interval_ms = (UsbConnected) ? 500 : 3000;
  if (millis() - start_ms < interval_ms)
    return;
  gpio_put(PICO_DEFAULT_LED_PIN, led_state);
  led_state = !led_state;
#endif

  start_ms += interval_ms;
}
#endif

void __time_critical_func(usb_core1init)()
{
#ifdef FUNCTION_LOGGING
  memset((void*)LogMemory, 0, sizeof(LogMemory));
#endif

#if 0
  // preload cache on CPU CORE1
  volatile uint8_t x = 0;
  for (uint32_t i=0;i<2048;i++)
  {
    x += MouseInterfaceROM[i];
  }
#endif
}

#ifdef DEBUG_OUTPUT
  static uart_inst_t *uart_inst;
#endif

/*------------- MAIN -------------*/
void usb_main(void)
{
#ifdef FUNCTION_LED
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif

#if defined(DEBUG_OUTPUT)
  #warning Only for debugging at the bench - without the Apple II connected...

  #define UART_DEV              PICO_DEFAULT_UART
  #define UART_TX_PIN           PICO_DEFAULT_UART_TX_PIN
  #define UART_RX_PIN           PICO_DEFAULT_UART_RX_PIN
  #define CFG_UART_BAUDRATE     115200

  //bi_decl(bi_2pins_with_func(UART_TX_PIN, UART_TX_PIN, GPIO_FUNC_UART));
  uart_inst = uart_get_instance(UART_DEV);
  stdio_uart_init_full(uart_inst, CFG_UART_BAUDRATE, UART_TX_PIN, UART_RX_PIN);
#endif

#ifdef DEBUG_OUTPUT
  printf("A2USB: "
 #ifdef FEATURE_CDC_SUPPORT
         "CDC "
 #endif
 #ifdef FEATURE_MSC_SUPPORT
         "MSC "
 #endif
         "HID "
         " SYS_CLOCK=%uMHz"
         "\r\n",
         CONFIG_SYSCLOCK);
#endif

  hid_app_init();

  // init host stack on configured roothub port
  tuh_init(BOARD_TUH_RHPORT);

  while (1)
  {
    // tinyusb host task
    tuh_task();

    // hid task
    hid_app_task();

#ifdef FEATURE_CDC_SUPPORT
    cdc_app_task();
#endif

#if 0 // keep these disabled - for now...
    // configuration commands
    config_handler();
#endif

#ifdef FUNCTION_LED
    usb_led_blinking();
#endif

  }
}

#endif // FUNCTION_USB
