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

/** PIA6520 internal register states */
typedef struct
{
  volatile uint8_t DDRA; /**< data direction register A */
  volatile uint8_t DDRB; /**< data direction register B */
  volatile uint8_t ORA;  /**< output register A */
  volatile uint8_t ORB;  /**< output register B */
  volatile uint8_t CRA;  /**< control register A */
  volatile uint8_t CRB;  /**< control register B */
  
  volatile uint64_t spacer; /**< spacer, separate Core 0+1 mem area */
  
  volatile uint8_t IA;  /**< input data port A */
  volatile uint8_t IB;  /**< input data port B */

#ifndef FEATURE_DISABLE_PIA_IRQS
  volatile uint8_t IRQA; /**< state of CA1/CA2 IRQ lines */
  volatile uint8_t IRQB; /**< state of CB1/CB2 IRQ lines */
#endif
} T6520;

/** Allow fast, direct access to the internal structure. */
extern T6520 Pia;

/** Read current PORTA output port (the physical output) */
#define PIA6520_PORTA() ((Pia.ORA & Pia.DDRA)|(Pia.IA & ~Pia.DDRA))

/** Read current PORTB output port (the physical output) */
#define PIA6520_PORTB() ((Pia.ORB & Pia.DDRB)|(Pia.IB & ~Pia.DDRB))

/** Provide new input data for external port A */
#define PIA6520_inputA(data) Pia.IA = data
/** Provide new input data for external port B */
#define PIA6520_inputB(data) Pia.IB = data;

void    PIA6520_write(uint32_t address, uint8_t data);
uint8_t PIA6520_read (uint16_t address);

#define PIA6520_fastwrite(address, data)\
{ \
    switch (address&3)\
    {\
        case 0:\
            if (Pia.CRA & 0x04) Pia.ORA  = data;else Pia.DDRA = data;\
            break;\
        case 1:\
            Pia.CRA  = data & 0x3f;\
            break;\
        case 2:\
            if (Pia.CRB & 0x04) Pia.ORB = data;else Pia.DDRB = data;\
            break;\
        case 3:\
            Pia.CRB  = data & 0x3f;\
            break;\
    }\
}
    
