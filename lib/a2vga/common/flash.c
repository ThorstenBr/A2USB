#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/watchdog.h>
#include <hardware/resets.h>
#include <hardware/dma.h>
#include <hardware/flash.h>
#include "common/config.h"
#include "common/buffers.h"
#include "common/flash.h"
#include <string.h>

#ifdef RASPBERRYPI_PICO_W
#include <pico/cyw43_arch.h>
#endif

void __time_critical_func(flash_reboot)() __attribute__ ((noreturn));

// Reboot the Pico
void __time_critical_func(flash_reboot)() {
    save_and_disable_interrupts();
    
    multicore_reset_core1();

    reset_block((1<<11) | (1<<10) | (1<<2));

    watchdog_enable(2, 1);
    for(;;);
}

#define CRC32_INIT                  ((uint32_t)-1l)
static uint8_t dummy_dst[1];

void __noinline __time_critical_func(flash_ota)() {
    // Get a free channel, panic() if there are none
    int chan = dma_claim_unused_channel(true);

    // 8 bit transfers. The read address increments after each transfer but
    // the write address remains unchanged pointing to the dummy destination.
    // No DREQ is selected, so the DMA transfers as fast as it can.
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // (bit-reverse) CRC32 specific sniff set-up
    channel_config_set_sniff_enable(&c, true);
    dma_sniffer_set_data_accumulator(CRC32_INIT);
    dma_sniffer_set_output_reverse_enabled(true);
    dma_sniffer_enable(chan, DMA_SNIFF_CTRL_CALC_VALUE_CRC32R, true);

    dma_channel_configure(
        chan,                   // Channel to be configured
        &c,                     // The configuration we just created
        dummy_dst,              // The (unchanging) write address
        (void*)FLASH_OTA_AREA,  // The initial read address
        FLASH_OTA_SIZE,         // Total number of transfers inc. appended crc; each is 1 byte
        true                    // Start immediately.
    );

    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    dma_channel_wait_for_finish_blocking(chan);

    uint32_t sniffed_crc = dma_sniffer_get_data_accumulator();
    if (0ul == sniffed_crc) {
        uint8_t *ptr = (uint8_t *)FLASH_OTA_AREA;
        uint32_t offset;
        flash_range_erase(0, FLASH_OTA_SIZE);

        // Copy from OTA area to Boot
        for(offset = 0; offset < FLASH_OTA_SIZE; offset+=65536) {
            memcpy((uint8_t *)private_memory, ptr+offset, 65536);
            flash_range_program(offset, (const uint8_t *)private_memory, 65536);
        }

        flash_range_erase((uint32_t)(FLASH_OTA_AREA-XIP_BASE), FLASH_OTA_SIZE);
        flash_reboot();
    }
}