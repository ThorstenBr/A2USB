/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2021-2022 Mark Aikens
 * Copyright (c) 2022-2023 David Kuder
 * Copyright (c) 2023-2024 Thorsten Brehm
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

#pragma once

#include <hardware/pio.h>
#include "common/abus.h"
#include "common/buffers.h"
#include "common/config.h"

#define A2VGA_SET_IRQ(state) gpio_put(CONFIG_PIN_IRQ, state)

static __always_inline bool A2VGA_INIT(void)
{
    abus_init();
    A2VGA_SET_IRQ(0);
    gpio_init(CONFIG_PIN_IRQ);
    gpio_set_dir(CONFIG_PIN_IRQ, GPIO_OUT);
    A2VGA_SET_IRQ(0);
}

#define A2VGA_GETADDRESS(value, address) \
        value = pio_sm_get_blocking(CONFIG_ABUS_PIO, ABUS_MAIN_SM); \
        address = (value >> 10) & 0xffff;

#define A2VGA_PUSHDATA(data) \
        CONFIG_ABUS_PIO->txf[ABUS_DEVICE_READ_SM] = data;

#define A2VGA_IS_SELECT()      (CARD_SELECT)
#define A2VGA_IS_DEVSEL()      (CARD_DEVSEL)
#define A2VGA_IS_IOSEL()       (CARD_IOSEL)
#define A2VGA_IS_ACCESS_READ() (ACCESS_READ)

static __always_inline bool A2VGA_IS_RESET(uint32_t value)
{
    switch(reset_state)
    {
        case 0:
            if((value & 0x7FFFF00) == ((0xFFFC << 10) | 0x300))
            {
                reset_state++;
                return false;
            }
            break;
        case 1:
            if((value & 0x7FFFF00) == ((0xFFFD << 10) | 0x300))
            {
                 reset_state++;
                 return false;
            }
            break;
        case 2:
            if((value & 0x7FFFF00) == ((0xFA62 << 10) | 0x300))
            {
                reset_state++;
                return true;
            }
            break;
        default:
            break;
    }
    reset_state = 0;
    return false;
}
