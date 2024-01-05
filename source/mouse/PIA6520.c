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

/* PIA6520.c: - Rockwell R6520 emulation.
   Copyright (c) 2024 Thorsten Brehm

   PIA registers (internal):
    DDRA  Data Direction Register A (1:output, 0:input)
    DDRB  Data Direction Register B (1:output, 0:input)
    CRA   Control Register A
    CRB   Control Register B
    ORA   Output Register A
    ORA   Output Register B

   Hardware Ports:
    PortA: (ORA & DDRA) | (InputA & ~DDRA); Bits reflect value of ORA when respective DDRA bit is set, otherwise acts as input.
    PortB: (ORB & DDRB) | (InputB & ~DDRB); Bits reflect value of ORB when respective DDRB bit is set, otherwise acts as input.

   PIA Adresses (2 bit address):
    0: PIBA: reads/writes DDRA (when CRA#2=0), reads PortA (when CRA#2=1). Writes ORA (when CRA#2=1).
    1: CRA
    2: PIBB: reads/writes DDRB (when CRB#2=0), reads PortB (when CRB#2=1). Writes ORB (when CRB#2=1).
    3: CRB
*/

#include "PIA6520.h"

#if 1
  #define SHOW_PIA()
#else
  #warning DEBUGGING ENABLED!
  #define SHOW_PIA() {\
    printf("PIA: DDR[%02x %02x] OR[%02x %02x] IR[%02x %02x]\n",\
           Pia.DDRA, Pia.DDRB, Pia.ORA, Pia.ORB, Pia.IA, Pia.IB); \
  }
#endif

/* The Apple Mouse Interface Card isn't using the PIA's IRQs.
 * The PIA's IRQ output wasn't even connected to anything on the
 * Apple II Mouse Interface Card.
 * We can keep this feature disabled entirely, improving performance. */
#define FEATURE_DISABLE_PIA_IRQS

/* PIA6520 IRQ flags */
#define PIA_IRQ1  0x80 /**< CA1/CB1 state */
#define PIA_IRQ2  0x40 /**< CA2/CB2 state */

/** PIA6520 IRQ selection */
#define IRQA1 0
#define IRQA2 1
#define IRQB1 2
#define IRQB2 3

/** Macro to clamp register address */
#define CLAMP_ADDRESS(address) { address &= 0x3; }

/** Internal state of the PIA registers */ 
T6520 Pia;

/** Initialize the PIA module at startup. */
static void __time_critical_func(PIA6520_init)(void)
{
    // wipe internal state
    //memset(&Pia, 0, sizeof(Pia));
    *((uint64_t*)&Pia) = 0;
    *((uint16_t*)&Pia.IA) = 0;
#ifndef FEATURE_DISABLE_PIA_IRQS
    *((uint16_t*)&Pia.IRQA) = 0;
#endif
}

#ifndef FEATURE_DISABLE_PIA_IRQS
/** Trigger a PIA IRQ, given by irq number (IRQA1,...,IRQB2) */
static void PIA6520_irq(uint8_t irq)
{
    // set interrupt request
    switch (irq)
    {
        case IRQA1:
            Pia.IRQA |= PIA_IRQ1;
	    break;
	case IRQA2:
            Pia.IRQA |= PIA_IRQ2;
            break;
        case IRQB1:
            Pia.IRQB |= PIA_IRQ1;
	    break;
	case IRQB2:
            Pia.IRQB |= PIA_IRQ2;
            break;
    }
}
#endif

/** Write to a PIA register. */
void __time_critical_func(PIA6520_write)(uint32_t address, uint8_t data)
{
    CLAMP_ADDRESS(address);
    //DEBUG_PRINT("PIA WRITE: %02x = %02x\n", address, data);
    switch (address)
    {
        case 0:
            if (Pia.CRA & 0x04)
                Pia.ORA  = data;
            else
                Pia.DDRA = data;
            break;
        case 1:
            // CA1/CA2 bits in control register are read-only
            data &= 0x3f;
            Pia.CRA  = data;
        #ifndef FEATURE_DISABLE_PIA_IRQS
            data |= Pia.IRQA;
        #endif
            break;
        case 2:
            if (Pia.CRB & 0x04)
                Pia.ORB = data;
            else
                Pia.DDRB = data;
            break;
        case 3:
            // CB1/CB2 bits in control register are read-only
            data &= 0x3f;
            Pia.CRB  = data;
        #ifndef FEATURE_DISABLE_PIA_IRQS
            data |= Pia.IRQB;
        #endif
            break;
    }
}

/** Read a PIA register. Normal read - calculated on the fly... */
uint8_t __time_critical_func(PIA6520_read)(uint16_t address)
{
    uint8_t temp;
    CLAMP_ADDRESS(address);
    switch (address)
    {
        case 0:
            return (Pia.CRA & 0x04) ? ((Pia.ORA & Pia.DDRA)|(Pia.IA & ~Pia.DDRA)) : Pia.DDRA;
        case 1:
            temp =  Pia.CRA;
        #ifndef FEATURE_DISABLE_PIA_IRQS
            temp |= Pia.IRQA;
            Pia.IRQA = 0;
        #endif
            return temp;
        case 2:
            return (Pia.CRB & 0x04) ? ((Pia.ORB & Pia.DDRB)|(Pia.IB & ~Pia.DDRB)) : Pia.DDRB;
        default:
        case 3:
            temp =  Pia.CRB;
        #ifndef FEATURE_DISABLE_PIA_IRQS
            temp |= Pia.IRQB;
            Pia.IRQB = 0;
        #endif
            return temp;
    }
}
