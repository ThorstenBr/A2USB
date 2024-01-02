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

/* 
   MouseInterfaceCard.c: Apple II Mouse Interface Card emulation.
   
   Copyright (c) 2024 Thorsten Brehm

   How the Apple II Mouse Interface Card works:
   
   The original Mouse Interface Card consists of a R6520/R6521/MC6821 PIA interface controller (all variants existed
   and exact type doesn't matter), and a MC6805 micro controller.
   
   The 6805 micro controller directly monitored the signals at the mouse connector, recording movement and button states.

   Protocol:
     The 6502 and 6805 communicate with a simple protocol. Data is exchanged via the PIA's port A.
     Each command has of 1-5 bytes. Length depends on the command ID, which is in the top 4 bits of the first byte sent.
     The initial command byte may be followed additional parameter bytes.
     All commands are written from the 6502=>6805. Some commands reply with 1-5 bytes of data.

     Handshaking is based on bits in PIA Port B:

       Write Transfers (6502=>6805) for commands + parameter bytes:
        - 6502 sets WRREQUEST=1 and waits until WRACK=1.
        - 6805 when seeing WRREQUEST=1: reads data from PIA Port A, then sets WRACK=1.
        - 6502 sets WRREQUEST=0 and waits until WRACK=0.
        - 6805 clears WRACK.

       Read Transfer (6502=>6805) for replies to certain commands:
        - 6805 sets RDREADY when data is availble to read (in response to a command).
        - 6502 waits until RDREADY=1 (after sending a command which responds with data).
        - 6502 reads data from PIA Port A.
        - 6502 sets RDACK and waits until RDREADY=0.
        - 6502 clears RDACK.
     
      As usual (for 6502 code), there were no timeouts. The MouseCard ROM routines get stuck if the
      6805 does not respond and handshake as expected.

   The 6805 controls the Apple II bus IRQ line directly. The PIA's IRQ mechanism is NOT used.
   The PCB does not even connect the PIA's own IRQ output to the Apple II bus.
   This can be confusing, since the PIA's own IRQ status and enable bits are completely irrelevant.
   
   The card has a 2KB ROM. It is mapped to the card's 256 byte slot ROM area, divided into 8 pages.
   The 8 slot ROM pages are selected through PIA port B (bits 1-2).
   (The code to switch slot ROM pages is placed at offset $xx70 of each 256 page, which allows to
   switch pages while code in the slot ROM itself is executed, and execution just continues on the
   newly selected page.)
   
   ROM initialization routine sets PORTB DDR to 0x3E (bits 1-5 as output, bits 0,6,7 as inputs).

   Mouse Commands:
      0x0n: SETMOUSE    Sets mouse mode. Mode in bits 0-3.
      0x1n: READMOUSE   Read current mouse status. Bits 0-3 ignored.
                        Provides 5 bytes to read: X position (2byte), Y position (2byte), Mouse Status.
      0x2n: SERVEMOUSE  Update status byte after IRQ. Bits 0-3 ignored.
      0x3n: CLEARMOUSE  Set mouse XY position to 0. Button and IRQ status unchanged. Bits 0-3 ignored.
      0x4n: POSMOUSE    Set mouse XY position to given values. Bits 0-3 ignored.
      0x5n: INITMOUSE   Initialize mouse to internal defaults. Bits 0-3 ignored.
      0x6n: CLAMPMOUSE  Set new boundaries for mouse position data. Bits 1-3 ignored. Bit 0: set X clamp (=0), or Y clamp (=1).
      0x7n: HOMEMOUSE   Set mouse to upper left corner of clamp window. Bits 0-3 ignored.
      0x8n: ?
      0x9n: TIMEMOUSE   Set 50Hz (bit 0=1) or 60Hz(bit0=0) IRQ mode. Bits 2,3: more bytes.
      0xAn: UNKNOWN_A0. Implemented by routine at offset $Cn1D. Expects one extra byte.
      0xBn: UNKNOWN_B0. Implemented by routine at offset $Cn1E.
      0xCn: UNKNOWN_C0. Implemented by routine at offset $Cn1F.
      0xDn: Reserved/unused.
      0xEn: Reserved/unused.
      0xFn: UNKNOWN_F0. Implemented by routine at offset $Cn1A. Expects a bit of data in A/bit 0.

   Mouse Operating Modes:
    Bit 0: Turn mouse on
        1: Enable interrupts on mouse movement
        2: Enable interrupts when button pressed
        3: Enable interrupts every screen refresh (50Hz/60Hz)
      4-7: Reserved
 
   Mouse Status Byte:
    Bit 0: Reserved (previous state of button 1)
        1: Interrupt caused by mouse movement
        2: Interrupt caused by button press
        3: Interrupt caused by screen refresh
        4: Reserved (current state of button 1)
        5: X or Y changed since last reading
        6: Button 0 was down at last reading
        7: Button 0 is down
        
   PIA Port A:
      8 bit communication to 6805 slave microcontroller

   PIA Port B:
     Bit 0: 6502=>PAL   Synchronization latch bit to sync counter in PAL.
         1: 6502=>ROM   SlotROM page selection, bit 0, ROM A8.
         2: 6502=>ROM   SlotROM page selection, bit 1, ROM A9.
         3: 6502=>ROM   SlotROM page selection, bit 2, ROM A10.
         4: 6502=>6805  RDACK (=1 acknowledged read).                                      
         5: 6502=>6805  WRREQUEST (=1 requests write). Slave accepts data on falling-edge.
         6: 6502<=6805  RDREADY. Slave has data available to read (=1).
         7: 6502<=6805  WRACK. Slave is has accepted write-request (=1).

*/

#ifdef FUNCTION_MOUSE

#include <string.h>
#ifdef PICO_BUILD
  #include <pico/stdlib.h>
  #include <pico/multicore.h>
  #include "common/config.h"
#endif

// include the ROM image here
#include "MouseInterfaceROM.h"

#if 0
  #warning DEBUG MODE enabled!
  #define DEBUG_PRINT printf
#else
  #define DEBUG_PRINT(...)
#endif

#ifdef PICO_BUILD
    /** macro to set IRQ pin */
    #define IRQ_ASSERT()    gpio_put(CONFIG_PIN_IRQ, 1)
    /** macro to clear IRQ pin */
    #define IRQ_DEASSERT()  gpio_put(CONFIG_PIN_IRQ, 0)
#endif

// inline the PIA emulation module for better performance
#include "PIA6520.c"

#define PIA_PORTB_RDACK     0x10
#define PIA_PORTB_WRREQUEST 0x20
#define PIA_PORTB_RDREADY   0x40
#define PIA_PORTB_WRACK     0x80

/* Mouse Commands */
#define COMMAND_SETMOUSE    0x00
#define COMMAND_READMOUSE   0x10
#define COMMAND_SERVEMOUSE  0x20
#define COMMAND_CLEARMOUSE  0x30
#define COMMAND_POSMOUSE    0x40
#define COMMAND_INITMOUSE   0x50
#define COMMAND_CLAMPMOUSE  0x60
#define COMMAND_HOMEMOUSE   0x70
#define COMMAND_80          0x80 // unknown command
#define COMMAND_TIMEMOUSE   0x90
#define COMMAND_A0          0xA0 // unknown command
#define COMMAND_B0          0xB0 // unknown command
#define COMMAND_C0          0xC0 // unknown command
#define COMMAND_D0          0xD0 // not implemented in 6805
#define COMMAND_E0          0xE0 // not implemented in 6805
#define COMMAND_F0          0xF0 // unknown command

/* Mouse Status */
#define STATUS_WAS_BUTTON1    (1<<0)
#define STATUS_IRQ_MOVEMENT   (1<<1)
#define STATUS_IRQ_BUTTON     (1<<2)
#define STATUS_IRQ_VBL        (1<<3)
#define STATUS_IS_BUTTON1     (1<<4)
#define STATUS_MOVED          (1<<5)
#define STATUS_WAS_BUTTON0    (1<<6)
#define STATUS_IS_BUTTON0     (1<<7)

/* Mouse Operating Mode */
#define MOUSE_MODE_ENABLED    (1<<0)
#define MOUSE_MODE_MOVED_IRQ  ((1<<1)|MOUSE_MODE_ENABLED)
#define MOUSE_MODE_BUTTON_IRQ ((1<<2)|MOUSE_MODE_ENABLED)
#define MOUSE_MODE_VBL_IRQ    ((1<<3)|MOUSE_MODE_ENABLED)

/* VBL Timeouts */
#define VBL_TIMER_60HZ     16666 /*us, = 16.666ms, for NTSC */
#define VBL_TIMER_50HZ     20000 /*us, for PAL*/
#define VBL_TIMER_DEFAULT  VBL_TIMER_60HZ

typedef struct
{
    uint8_t Command;        /**< Current command byte. */
    uint8_t ReadBuffer[8];  /**< Read buffer:  data returned from 6805 mouse slave controller to 6502. */
    uint8_t WriteBuffer[8]; /**< Write buffer: data from 6502 sent to 6805 mouse slave controller. */
    uint8_t ReadPos;
    uint8_t WritePos;
    uint8_t LastPortB;

    bool     Vbl50HzMode;
    uint64_t VblTimeUs;
    uint32_t VblIntervalUs;

    uint8_t  OperatingMode;
    uint8_t  IntState;

    struct
    {
        uint16_t X;
        uint16_t Y;
        bool     Button0;
        bool     Button1;
    } Current;

    struct
    {
        uint16_t X;
        uint16_t Y;
        bool     Button0;
        bool     Button1;
    } Last;

    struct
    {
        uint16_t MinX;
        uint16_t MinY;
        uint16_t MaxX;
        uint16_t MaxY;
    } Clamp;

} TA2Mouse;

TA2Mouse Mouse;

static void clampXY()
{
    if (Mouse.Current.X < Mouse.Clamp.MinX)
        Mouse.Current.X = Mouse.Clamp.MinX;
    if (Mouse.Current.Y < Mouse.Clamp.MinY)
        Mouse.Current.Y = Mouse.Clamp.MinY;
    if (Mouse.Current.X > Mouse.Clamp.MaxX)
        Mouse.Current.X = Mouse.Clamp.MaxX;
    if (Mouse.Current.Y > Mouse.Clamp.MaxY)
        Mouse.Current.Y = Mouse.Clamp.MaxY;
}

static void mouseCommandSet()
{
    Mouse.OperatingMode = Mouse.Command & 0x0F;
    //DEBUG_PRINT("MOUSE: OPERATING MODE=%02x  %s\n", Mouse.OperatingMode, (Mouse.OperatingMode&1) ? "ENABLED" : "DISABLED");
}

static void mouseCommandRead()
{
    uint8_t IntState = Mouse.IntState & STATUS_MOVED;

    if (Mouse.Last.Button0)
        IntState |= STATUS_WAS_BUTTON0;
    if (Mouse.Last.Button1)
        IntState |= STATUS_WAS_BUTTON1;
    if (Mouse.Current.Button0)
        IntState |= STATUS_IS_BUTTON0;
    if (Mouse.Current.Button1)
        IntState |= STATUS_IS_BUTTON1;

    // prepare return buffer
    Mouse.ReadBuffer[4] =  Mouse.Current.X       & 0xff;
    Mouse.ReadBuffer[3] = (Mouse.Current.X >> 8) & 0xff;
    Mouse.ReadBuffer[2] =  Mouse.Current.Y       & 0xff;
    Mouse.ReadBuffer[1] = (Mouse.Current.Y >> 8) & 0xff;
    Mouse.ReadBuffer[0] = IntState;

    Mouse.IntState = IntState & ~STATUS_MOVED;

    memcpy(&Mouse.Last, &Mouse.Current, sizeof(Mouse.Current));

    Mouse.ReadPos = 5; // 5 bytes to be read
}

static void mouseCommandServe()
{
   Mouse.ReadBuffer[0] = Mouse.IntState & ~(1<<5); // except for bit 5 (X or Y changed since last reading)
   // 1 byte to be read
   Mouse.ReadPos = 1;
   // clear IRQ requests
   Mouse.IntState &= ~(STATUS_IRQ_VBL|STATUS_IRQ_MOVEMENT|STATUS_IRQ_BUTTON);
   IRQ_DEASSERT();
}

static void mouseCommandClear()
{
    // CLEARMOUSE sets the mouse's X and Y position values to 0.
    // The button and interrupt status byte remains unchanged.
    Mouse.Current.X = Mouse.Current.Y = 0;
}

static void mouseCommandPos()
{
    Mouse.Current.X = Mouse.WriteBuffer[0] | (Mouse.WriteBuffer[1] << 8);
    Mouse.Current.Y = Mouse.WriteBuffer[2] | (Mouse.WriteBuffer[3] << 8);
    clampXY();
    Mouse.Last.X = Mouse.Current.X;
    Mouse.Last.Y = Mouse.Current.Y;
}

static void mouseCommandHome()
{
    Mouse.Current.X = Mouse.Last.X = Mouse.Clamp.MinX;
    Mouse.Current.Y = Mouse.Last.Y = Mouse.Clamp.MinY;
}

static void mouseCommandInit()
{
    Mouse.Clamp.MaxX = Mouse.Clamp.MaxY = 1023;
    Mouse.Clamp.MinX = Mouse.Clamp.MinY = 0;
    Mouse.VblTimeUs = 0;
    mouseCommandHome();
    IRQ_DEASSERT();
}

static void mouseCommandClamp()
{
    uint16_t MinClamp = Mouse.WriteBuffer[3] | (Mouse.WriteBuffer[1] << 8);
    uint16_t MaxClamp = Mouse.WriteBuffer[2] | (Mouse.WriteBuffer[0] << 8);

    if (MinClamp > MaxClamp)
    {
        uint32_t t = MaxClamp;
        t += MinClamp;
        MaxClamp = (t>>1);
        MinClamp = 0;
    }

    if (Mouse.Command & 0x1)
    {
       // set clamp Y
       Mouse.Clamp.MinY = MinClamp;
       Mouse.Clamp.MaxY = MaxClamp;
    }
    else
    {
       // set clamp X
       Mouse.Clamp.MinX = MinClamp;
       Mouse.Clamp.MaxX = MaxClamp;
    }
    clampXY();
}

static void mouseCommandTime()
{
    Mouse.Vbl50HzMode = (Mouse.Command & 0x1); // bit 0: 1=50Hz, 0=60Hz
    // TODO Vbl interval is not exact enough. Should be synchronized with 6502 CPU clock anyway.
    Mouse.VblIntervalUs = (Mouse.Vbl50HzMode) ? VBL_TIMER_50HZ : VBL_TIMER_60HZ;
}

static void mouseCommand(void)
{
    switch(Mouse.Command & 0xF0)
    {
        case COMMAND_SETMOUSE:    mouseCommandSet();   break;
        case COMMAND_READMOUSE:   mouseCommandRead();  break;
        case COMMAND_SERVEMOUSE:  mouseCommandServe(); break;
        case COMMAND_CLEARMOUSE:  mouseCommandClear(); break;
        case COMMAND_POSMOUSE:    mouseCommandPos();   break;
        case COMMAND_INITMOUSE:   mouseCommandInit();  break;
        case COMMAND_CLAMPMOUSE:  mouseCommandClamp(); break;
        case COMMAND_HOMEMOUSE:   mouseCommandHome();  break;
        case COMMAND_TIMEMOUSE:   mouseCommandTime();  break;
        case COMMAND_80:          /*?*/                break;
        case COMMAND_A0:          /*?*/                break;
        case COMMAND_B0:          /*?*/                break;
        case COMMAND_C0:          /*?*/                break;
        case COMMAND_F0:          /*?*/                break;
        default:                  /*?*/                break;
    }
}

static void mouseControllerAcceptData(void)
{
    // fallinge-edge detected
    if (Mouse.WritePos)  // we were waiting for more parameter data
        Mouse.WriteBuffer[--Mouse.WritePos] = PIA6520_PORTA();
    else
    {
        // no parameters pending: new command starting...
        Mouse.Command = PIA6520_PORTA();

        // now let's see how many parameters we need for this command
        switch(Mouse.Command & 0xF0)
        {
            case COMMAND_POSMOUSE:    Mouse.WritePos = 4; break; // 4 parameter bytes
            case COMMAND_INITMOUSE:                       break; // no parameter bytes
            case COMMAND_CLAMPMOUSE:  Mouse.WritePos = 4; break; // 4 parameter bytes
            case COMMAND_A0:          Mouse.WritePos = 1; break; // 1 parameter byte (no clue what is being sent)
            case COMMAND_TIMEMOUSE:
                switch(Mouse.Command & 0xc)
                {
                    case 0x4:         Mouse.WritePos = 2; break; // 2 parameter bytes
                    case 0x8:         Mouse.WritePos = 1; break; // 1 parameter byte
                    case 0xc:         Mouse.WritePos = 3; break; // 3 parameter bytes
                    default:                              break; // no parameter bytes
                }
                break;
            default:                                             // no parameter bytes
                /* includes COMMAND_SETMOUSE,COMMAND_HOMEMOUSE,COMMAND_READMOUSE,
                            COMMAND_SERVEMOUSE,COMMAND_CLEARMOUSE*/
                break;
        }
    }
    // remaining parameter bytes?
    if (Mouse.WritePos == 0)
    {
        // new command is complete
        mouseCommand();
    }
}

/** Process write data: returns true when a new command is available */
static void mouseControllerWrite(uint8_t portB)
{
    // write
    if (portB & PIA_PORTB_WRREQUEST)
    {
        if (Mouse.ReadPos)
        {
            DEBUG_PRINT("MOUSE: %u bytes of data were not retrieved.\n", Mouse.ReadPos);
        }

        // clears pending output data
        Mouse.ReadPos = 0;

        mouseControllerAcceptData();

        // acknowledge data from 6502
        PIA6520_inputB((Pia.IB & ~PIA_PORTB_RDREADY) | PIA_PORTB_WRACK);
    }
    else
    {
        // clear acknowledge
        if (Pia.IB & PIA_PORTB_WRACK)
            PIA6520_inputB(Pia.IB & ~PIA_PORTB_WRACK);
    }
}

/** Process read data: returns the read buffer content to the 6502 */
static void mouseControllerRead(uint8_t portB)
{
    // read acknowledge?
    if (portB & PIA_PORTB_RDACK)
    {
        if (Pia.IB & PIA_PORTB_RDREADY)
        {
            //DEBUG_PRINT("mouseControllerRead: read complete, ReadPos=%u\n",Mouse.ReadPos);
            if (Mouse.ReadPos > 0)
                Mouse.ReadPos--;
            else
            {
                DEBUG_PRINT("MOUSE: Unexpected read. Command=%02x\n", Mouse.Command);
            }
            // clear read-ready flag
            PIA6520_inputB(Pia.IB & ~PIA_PORTB_RDREADY);
        }
    }
    else
    {
        /* Normally we would only need to indicate RDREADY when we have data available after a command.
           However, to avoid issues with the ROM getting stuck, we always indicated "RDREADY", unless
           a write operation is currently pending. This avoids issues when the ROM firmware was making an
           'unexpected read' (since there are no timeouts, "of course", so if it got stuck, then it'd be
           stuck forever...).
         */
        //if (Mouse.ReadPos > 0)
        if ((portB & (PIA_PORTB_WRACK|PIA_PORTB_WRREQUEST))==0)
        {
            if ((Pia.IB & PIA_PORTB_RDREADY) == 0)
            {
               //DEBUG_PRINT("mouseControllerRead: read, ReadPos=%u\n", Mouse.ReadPos);
               // set port A to data
               PIA6520_inputA((Mouse.ReadPos>0) ? Mouse.ReadBuffer[Mouse.ReadPos-1] : 0x00);
               // read data is ready
               PIA6520_inputB(Pia.IB | PIA_PORTB_RDREADY);
            }
        }
    }
}

/** Mouse movement reports are processed here. */
void mouseControllerMoveXY(int8_t X, int8_t Y)
{
    uint16_t OldX = Mouse.Current.X;
    uint16_t OldY = Mouse.Current.Y;

    // update current position, avoid over- and underflows, clamp to range
    if (X>0)
    {
        Mouse.Current.X += X;
        if ((Mouse.Current.X < OldX)||(Mouse.Current.X>Mouse.Clamp.MaxX))
            Mouse.Current.X = Mouse.Clamp.MaxX;
    }
    else
    {
        Mouse.Current.X += X;
        if ((Mouse.Current.X > OldX)||(Mouse.Current.X<Mouse.Clamp.MinX))
            Mouse.Current.X = Mouse.Clamp.MinX;
    }
    if (Y>0)
    {
        Mouse.Current.Y += Y;
        if ((Mouse.Current.Y < OldY)||(Mouse.Current.Y>Mouse.Clamp.MaxY))
            Mouse.Current.Y = Mouse.Clamp.MaxY;
    }
    else
    {
        Mouse.Current.Y += Y;
        if ((Mouse.Current.Y > OldY)||(Mouse.Current.Y<Mouse.Clamp.MinY))
            Mouse.Current.Y = Mouse.Clamp.MinY;
    }

    // was there any actual movement?
    if ((Mouse.Current.X != OldX)||
        (Mouse.Current.Y != OldY))
    {
        Mouse.IntState |= STATUS_MOVED;

        // movement interrupt enabled?
        if ((Mouse.OperatingMode & MOUSE_MODE_MOVED_IRQ) == MOUSE_MODE_MOVED_IRQ)
            Mouse.IntState |= STATUS_IRQ_MOVEMENT;
    }
}

/** Mouse button reports are processed here. */
void mouseControllerUpdateButton(uint8_t ButtonNr, bool Pressed)
{
    if (ButtonNr == 0)
        Mouse.Current.Button0 = Pressed;
    else
        Mouse.Current.Button1 = Pressed;

    // button interrupt enabled?
    if ((Mouse.OperatingMode & MOUSE_MODE_BUTTON_IRQ) == MOUSE_MODE_BUTTON_IRQ)
    {
        Mouse.IntState |= STATUS_IRQ_BUTTON;
    }
}

/** Simple VBL IRQ generation. This needs to be improved to be really synchronous to the 6502 VBL. */
static void mouseControllerVblIrq(void)
{
    // Vertical BLanking interrupt enabled?
    if ((Mouse.OperatingMode & MOUSE_MODE_VBL_IRQ) == MOUSE_MODE_VBL_IRQ)
    {
        // we're currently not really synchronous with the CRT blanking. Just trigger at 60Hz/50Hz
        // using internal PICO timer.
        uint64_t t = get_absolute_time();
        if (Mouse.VblTimeUs == 0)
            Mouse.VblTimeUs = t+Mouse.VblIntervalUs; // restart the timer calculation
        else
        if (t >= Mouse.VblTimeUs)
        {
            // trigger IRQ
            Mouse.IntState |= STATUS_IRQ_VBL;

            /* Advance timer by interval. We do not reset it according to current time, to avoid adding up
               timing errors, since we will always be a bit late for each IRQ. */
            Mouse.VblTimeUs += Mouse.VblIntervalUs;
        }
    }
    else
    {
       // VBL IRQ is disabled. Clear timeout.
       Mouse.VblTimeUs = 0;
    }
}

void mouseControllerRun(void)
{
    uint8_t PortB = PIA6520_PORTB();
    static uint8_t OldInt = 0;

    // any write operations?
    if ((PortB ^ Mouse.LastPortB) & PIA_PORTB_WRREQUEST)
    {
        mouseControllerWrite(PortB);
    }

    // process read requests
    mouseControllerRead(PortB);
    Mouse.LastPortB = PortB;

    // generate VBL interrupts
    mouseControllerVblIrq();

    // finally, do we need to trigger the AppleIIBus IRQ line?
    if (Mouse.IntState & (STATUS_IRQ_VBL|STATUS_IRQ_MOVEMENT|STATUS_IRQ_BUTTON))
    {
        if ((OldInt & (STATUS_IRQ_VBL|STATUS_IRQ_MOVEMENT|STATUS_IRQ_BUTTON)) == 0)
        {
            IRQ_ASSERT();
        }
    }
    OldInt = Mouse.IntState;
}

void __time_critical_func(mouseControllerReset)(void)
{
    PIA6520_init();

    // quick memory init
    for (uint32_t i=0;i<sizeof(Mouse)/8;i++)
    {
        ((volatile uint64_t*)&Mouse)[i] = 0;
    }
    Mouse.Clamp.MaxX = 1023;
    Mouse.Clamp.MaxY = 1023;
    Mouse.VblIntervalUs = VBL_TIMER_DEFAULT;
}

void mouseControllerInit(void)
{
#ifdef PICO_BUILD
    // prepare PICO IRQ output pin
    static bool done=false;
    if (!done)
    {
      done = true;
      IRQ_DEASSERT();
      gpio_init(CONFIG_PIN_IRQ);
      gpio_set_dir(CONFIG_PIN_IRQ, GPIO_OUT);
    }
    IRQ_DEASSERT();
#endif
    mouseControllerReset();
}

#endif // FUNCTION_MOUSE
