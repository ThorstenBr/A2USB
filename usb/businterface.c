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

#include <string.h>
#include <hardware/pio.h>
#include "common/config.h"
#include "common/buffers.h"
#include "common/abus.h"
#include "usb/usb.h"

#ifdef FUNCTION_MOUSE
  #include "mouse/MouseInterfaceCard.h"
#endif

#ifdef FUNCTION_LOGGING
uint32_t LogCounter = 0; // position of recording 
uint32_t LogOffset  = 0; // position of viewer
uint32_t LogTrigger = 0; // trigger state (0=OFF, 1=waiting, 2=recording)
#endif

/** The offset to the currently seleced page in the MouseInterface SlotROM. */
uint32_t ROMOffset = 0;

#ifdef FUNCTION_ROM_WRITE
/** Debug switch to allow writing to SlotROM for debugging. */
uint8_t  ROMWriteEnable = 0;
#endif

void __time_critical_func(usb_buswrite)(uint32_t address, uint32_t value)
{
#ifdef FUNCTION_MOUSE
    if(CARD_DEVSEL)
    {
  #ifdef FUNCTION_LOGGING
        if ((address&0x7)==7)
        {
           value &= 0xff;
           if (value == 0xFF)
           {
             LogCounter = 0;
             LogTrigger = 1;
           }
           else
           if (value >= 0xFC)
           {
               LogTrigger = (value&0x3);
           }
  #ifdef FUNCTION_ROM_WRITE
           else
           if (value == 0xAA)
           {
               ROMWriteEnable = 1;
           }
  #endif // FUNCTION_ROM_WRITE
           else
           if (LogCounter)
           {
             LogTrigger = 0;
             LogOffset  = (value << 8)|(0x10000);
           }
        }
        else
  #endif // FUNCTION_LOGGING
        {
          // PIA registers are being written
          // PIA6520_fastwrite(address,value);
          // code just inlined here - so we can add debug hooks...
          switch (address&3)
          {
            case 0:
                if (Pia.CRA & 0x04) Pia.ORA = value;else Pia.DDRA = value;
                break;
            case 1:
                Pia.CRA  = value & 0x3f;
                break;
            case 2:
                if (Pia.CRB & 0x04) Pia.ORB = value;else Pia.DDRB = value;
                // prepare ROMOffset - so we don't need to do that in a tight read-cycle
                ROMOffset = ((Pia.ORB & Pia.DDRB & 0x0E)<<7);
  #ifdef FUNCTION_LOGGING
                // special log event when the SlotROM page was switched
                if ((LogTrigger==2) && ((LogCounter & 0x4000)==0))
                  LogMemory[LogCounter++] = 0xffff00ff | ROMOffset;
  #endif
                break;
            case 3:
                Pia.CRB  = value & 0x3f;
                break;
          }
        }
    }
 #if FUNCTION_ROM_WRITE // ROM Write Enable
    else
    if (CARD_IOSEL)
    {
        // we ignore WRITEs to the ROM area for now (we may add config update support later)
        if (ROMWriteEnable)
        {
          MOUSE_INTERFACE_PROGRAM_ROM(address, value);
        }
    }
 #endif // FUNCTION_ROM_WRITE
#endif // FUNCTION_MOUSE
}

uint8_t __time_critical_func(usb_busread)(uint32_t address)
{
#ifdef FUNCTION_MOUSE
    // our slot's DEVSELECT or IOSELECT is active
    if(CARD_DEVSEL)
    {
        // PIA registers are being read
        CONFIG_ABUS_PIO->txf[ABUS_DEVICE_READ_SM] = PIA6520_read(address);
    }
    else
    if (CARD_IOSEL)
    {
 #ifdef FUNCTION_LOGGING
        address &= 0xff;
        if (LogOffset)
        {
          address |= (LogOffset&0xffff);
          CONFIG_ABUS_PIO->txf[ABUS_DEVICE_READ_SM] = (address<(LogCounter<<2)) ? ((uint8_t*)LogMemory)[address] : 0;
        }
        else
        {
          CONFIG_ABUS_PIO->txf[ABUS_DEVICE_READ_SM] = MouseInterfaceROM[address | ROMOffset];
        }
 #else
        CONFIG_ABUS_PIO->txf[ABUS_DEVICE_READ_SM] = MouseInterfaceROM[(address&0xff) | ROMOffset];
 #endif
    }
#endif
}

void __time_critical_func(usb_reset)()
{
  // Reset when the Apple II resets
  mouseControllerInit();
  ROMOffset = 0;
#ifdef FUNCTION_LOGGING
  // stop recording when RESET triggers
  LogTrigger = 0;
#endif
}
