/* 
 * The MIT License (MIT)
 *
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

#pragma once

#ifdef FUNCTION_MOUSE

#include "PIA6520.h"

/** Initialization at startup/reset */
extern void mouseControllerInit         (void);

/** Loop / Run method to process slave commands */
extern void mouseControllerRun          (void);

/** Report new mouse movement */
extern void mouseControllerMoveXY       (int8_t X, int8_t Y);

/** Report new button press/release */
extern void mouseControllerUpdateButton (uint8_t ButtonNr, bool Pressed);

/** Get byte from current ROM page */
extern volatile uint8_t MouseInterfaceROM[]; // explicitly not "const": data needs to be in RAM for faster access

/** Read from the Slot ROM */
#define MOUSE_INTERFACE_READ_ROM(Address)           (MouseInterfaceROM[(Address&0xff) | ((Pia.ORB & Pia.DDRB & 0x0E)<<7)])

/** Program the Slot ROM */
#define MOUSE_INTERFACE_PROGRAM_ROM(Address, value) MouseInterfaceROM[(Address&0xff) | ((Pia.ORB & Pia.DDRB & 0x0E)<<7)] = value

#endif // FUNCTION_MOUSE
