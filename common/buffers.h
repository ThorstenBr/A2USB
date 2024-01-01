#pragma once

#include <stdint.h>

extern volatile uint8_t reset_state;

extern volatile uint8_t cardslot;
extern volatile uint32_t busactive;

extern volatile uint8_t apple_memory[64*1024];
#ifndef FUNCTION_USB
extern volatile uint8_t private_memory[64*1024];
#endif
#ifdef FUNCTION_LOGGING
extern volatile uint32_t LogMemory[16*1024];
#endif

extern volatile uint16_t cfptr;
extern volatile uint8_t cfbuf[4096];

#define config_cmdbuf ((uint8_t*)(apple_memory+0xC0F0+(cardslot<<8)))
#define config_rpybuf ((uint8_t*)(apple_memory+0xC0F8+(cardslot<<8)))

extern volatile uint8_t jumpers;

#ifdef FUNCTION_VGA
extern volatile uint8_t *text_p1;
extern volatile uint8_t *text_p2;
extern volatile uint8_t *text_p3;
extern volatile uint8_t *text_p4;
extern volatile uint8_t *hgr_p1;
extern volatile uint8_t *hgr_p2;
extern volatile uint8_t *hgr_p3;
extern volatile uint8_t *hgr_p4;

/* Videx VideoTerm */
extern volatile uint8_t *videx_page;

#define apple_tbcolor  apple_memory[0xC022]
#define apple_border   apple_memory[0xC034]

#endif

extern volatile uint8_t *baseio;
extern volatile uint8_t *slotio;
extern volatile uint8_t *slotrom;
extern volatile uint8_t *extdrom;

/* Slot 1: Grapper */
extern volatile uint8_t *slot1io;
extern volatile uint8_t *slot1rom;

/* Slot 2: Super Serial Card */
extern volatile uint8_t *slot2io;
extern volatile uint8_t *slot2rom;

/* Slot 3: 80 Column Card */
extern volatile uint8_t *slot3io;
extern volatile uint8_t *slot3rom;

/* Slot 4: PCPI Applicard */
extern volatile uint8_t *slot4io;
extern volatile uint8_t *slot4rom;

/* Slot 5: Uthernet */
extern volatile uint8_t *slot5io;
extern volatile uint8_t *slot5rom;

/* Slot 6: Disk II */
extern volatile uint8_t *slot6io;
extern volatile uint8_t *slot6rom;

/* Slot 7: ProDOS ROM Disk */
extern volatile uint8_t *slot7io;
extern volatile uint8_t *slot7rom;

extern volatile uint32_t soft_switches;

extern volatile uint32_t internal_flags;

#define SOFTSW_TEXT_MODE      0x00000001ul
#define SOFTSW_MIX_MODE       0x00000002ul
#define SOFTSW_HIRES_MODE     0x00000004ul
#define SOFTSW_MODE_MASK      0x00000007ul
#define SOFTSW_PAGE_2         0x00000008ul

// Apple IIe/c/gs softswitches
#define SOFTSW_80STORE        0x00000100ul
#define SOFTSW_AUX_READ       0x00000200ul
#define SOFTSW_AUX_WRITE      0x00000400ul
#define SOFTSW_AUXZP          0x00000800ul
#define SOFTSW_SLOT3ROM       0x00001000ul
#define SOFTSW_80COL          0x00002000ul
#define SOFTSW_ALTCHAR        0x00004000ul
#define SOFTSW_DGR            0x00008000ul

#define SOFTSW_NEWVID_MASK    0xE0ul
#define SOFTSW_NEWVID_SHIFT   11

#define SOFTSW_MONOCHROME     0x00010000ul
#define SOFTSW_LINEARIZE      0x00020000ul
#define SOFTSW_SHR            0x00040000ul

#define SOFTSW_IOUDIS         0x00080000ul

#define SOFTSW_SHADOW_MASK    0x7Ful
#define SOFTSW_SHADOW_SHIFT   20

#define SOFTSW_SHADOW_TEXT    0x00100000ul
#define SOFTSW_SHADOW_HGR1    0x00200000ul
#define SOFTSW_SHADOW_HGR2    0x00400000ul
#define SOFTSW_SHADOW_SHR     0x00800000ul
#define SOFTSW_SHADOW_AUXHGR  0x01000000ul
#define SOFTSW_SHADOW_ALTDISP 0x02000000ul
#define SOFTSW_SHADOW_IO      0x04000000ul

// V2 Analog specific softswitches
#define IFLAGS_INTERP         0x01000000ul
#define IFLAGS_GRILL          0x02000000ul
#define IFLAGS_VIDEO7         0x04000000ul
#define IFLAGS_OLDCOLOR       0x08000000ul
#define IFLAGS_TERMINAL       0x10000000ul
#define IFLAGS_TEST           0x20000000ul
#define IFLAGS_IIE_REGS       0x40000000ul
#define IFLAGS_IIGS_REGS      0x80000000ul

#define IFLAGS_V7_MODE0       0x00000000ul
#define IFLAGS_V7_MODE1       0x00000001ul
#define IFLAGS_V7_MODE2       0x00000002ul
#define IFLAGS_V7_MODE3       0x00000003ul
