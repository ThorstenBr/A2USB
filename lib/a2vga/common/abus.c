#include <string.h>
#include <hardware/pio.h>
#include "common/config.h"
#include "common/abus.h"

#ifdef OVERCLOCKED
 #ifdef ANALOG_GS
  #include "abus-gs-4ns.pio.h"
 #else
  #include "abus-4ns.pio.h"
 #endif
#else
 #ifdef ANALOG_GS
  #include "abus-gs-8ns.pio.h"
 #else
  #include "abus-8ns.pio.h"
 #endif
#endif

#if CONFIG_PIN_APPLEBUS_PHI0 != PHI0_GPIO
#error CONFIG_PIN_APPLEBUS_PHI0 and PHI0_GPIO must be set to the same pin
#endif

#if ((abus_wrap+1)+(abus_device_read_wrap+1))>32
#error The ABUS PIO programs exceed the capacity of the RP2040 PIO (>32 words).
#endif

static void abus_device_read_setup(PIO pio, uint sm) {
    uint program_offset = pio_add_program(pio, &abus_device_read_program);
    pio_sm_claim(pio, sm);

    pio_sm_config c = abus_device_read_program_get_default_config(program_offset);

    // set the "device selected" pin as the jump pin
    sm_config_set_jmp_pin(&c, CONFIG_PIN_APPLEBUS_DEVSEL);

    // map the OUT pin group to the data signals
    sm_config_set_out_pins(&c, CONFIG_PIN_APPLEBUS_DATA_BASE, 8);

    // map the SET pin group to the Data transceiver control signals
    sm_config_set_set_pins(&c, CONFIG_PIN_APPLEBUS_CONTROL_BASE, 2);

    pio_sm_init(pio, sm, program_offset, &c);

    // All the GPIOs are shared and setup by the main program
}

static void abus_main_setup(PIO pio, uint sm) {
    uint program_offset = pio_add_program(pio, &abus_program);
    pio_sm_claim(pio, sm);

    pio_sm_config c = abus_program_get_default_config(program_offset);

    // set the bus R/W pin as the jump pin
    sm_config_set_jmp_pin(&c, CONFIG_PIN_APPLEBUS_RW);

    // map the IN pin group to the data signals
    sm_config_set_in_pins(&c, CONFIG_PIN_APPLEBUS_DATA_BASE);

    // map the SET pin group to the bus transceiver enable signals
    sm_config_set_set_pins(&c, CONFIG_PIN_APPLEBUS_CONTROL_BASE+1, 3);

#ifdef ANALOG_GS
    // configure left shift into ISR & autopush every 32 bits
    sm_config_set_in_shift(&c, false, true, 32);
#else
    // configure left shift into ISR & autopush every 26 bits
    sm_config_set_in_shift(&c, false, true, 26);
#endif

    pio_sm_init(pio, sm, program_offset, &c);

    // configure the GPIOs
    // Ensure all transceivers will start disabled, with Data transceiver direction set to 'in'
    pio_sm_set_pins_with_mask(pio, sm,
        (uint32_t)0xe << CONFIG_PIN_APPLEBUS_CONTROL_BASE,
        (uint32_t)0xf << CONFIG_PIN_APPLEBUS_CONTROL_BASE);
    pio_sm_set_pindirs_with_mask(pio, sm,
        (0xf << CONFIG_PIN_APPLEBUS_CONTROL_BASE),
        (1 << CONFIG_PIN_APPLEBUS_PHI0) | (0xf << CONFIG_PIN_APPLEBUS_CONTROL_BASE) | (0x3ff << CONFIG_PIN_APPLEBUS_DATA_BASE));

    // Disable input synchronization on input pins that are sampled at known stable times
    // to shave off two clock cycles of input latency
    // (data bus + DEVSEL + RW)
    pio->input_sync_bypass |= (0x3ff << CONFIG_PIN_APPLEBUS_DATA_BASE);

    // configure GPIO for PHI0 input
    pio_gpio_init(pio, CONFIG_PIN_APPLEBUS_PHI0);
    gpio_set_pulls(CONFIG_PIN_APPLEBUS_PHI0, false, false);
    gpio_set_input_hysteresis_enabled(CONFIG_PIN_APPLEBUS_PHI0, true);

    // configure 4 GPIOs for transceiver control
    for(int pin=CONFIG_PIN_APPLEBUS_CONTROL_BASE; pin < CONFIG_PIN_APPLEBUS_CONTROL_BASE+4; pin++) {
        pio_gpio_init(pio, pin);
        gpio_set_slew_rate(pin, GPIO_SLEW_RATE_FAST);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
    }

    // configure 8 GPIOs for data bus + 2 GPIOs for DEVSEL/RW
    for(int pin=CONFIG_PIN_APPLEBUS_DATA_BASE; pin < CONFIG_PIN_APPLEBUS_DATA_BASE+10; pin++) {
        pio_gpio_init(pio, pin);
        gpio_set_pulls(pin, false, false);
        gpio_set_input_hysteresis_enabled(pin, false);
        if (pin < (CONFIG_PIN_APPLEBUS_DATA_BASE+8))
        {
            gpio_set_slew_rate(pin, GPIO_SLEW_RATE_FAST);
            gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_8MA);
        }
    }
}


void abus_init() {
    // configure state machine to write data for read-cycles to the 6502 bus
    abus_device_read_setup(CONFIG_ABUS_PIO, ABUS_DEVICE_READ_SM);
    // configure main state machine to monitor 6502 bus cycles
    abus_main_setup(CONFIG_ABUS_PIO, ABUS_MAIN_SM);

    pio_enable_sm_mask_in_sync(CONFIG_ABUS_PIO, (1 << ABUS_MAIN_SM) | (1 << ABUS_DEVICE_READ_SM));
}
