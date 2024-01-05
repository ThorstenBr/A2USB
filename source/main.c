/*
MIT License

Copyright (c) 2021-2022 Mark Aikens
Copyright (c) 2022-2023 David Kuder
Copyright (c) 2023-2024 Thorsten Brehm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <pico/stdlib.h>
#include <pico/multicore.h>

#include "a2platform.h"
#include "dma/dmacopy.h"
#include "util/profiler.h"
#include "util/logger.h"

#include "usb/usb.h"
#include "usb/businterface.c"

#ifdef FUNCTION_LOGGING
	#define LOGTRIGGER_STARTADDRESS 0xC400
	extern uint32_t LogTrigger;
#endif

#ifdef FUNCTION_PROFILER
	uint32_t ProfilerMaxTime;
#endif

static __always_inline void sys_reset(void)
{
  // Reset when the Apple II resets
  mouseControllerReset();
  ROMOffset = 0;

  // stop logging on 6502 HW reset
  LOGGER_STOP();
}

static void __noinline __time_critical_func(core1_loop)()
{
    uint32_t value;
    uint32_t address;

#ifdef FUNCTION_LOGGING
  memset((void*)LogMemory, 0, sizeof(LogMemory));
#endif

    // enable systick timer, but keep timer exception disabled
    PROFILER_INIT(ProfilerMaxTime);

    for(;;)
    {
        // wait for next PIO event
        A2_GETADDRESS(value, address);

        // start time measurement (count-down timer)
        PROFILER_START();

        if(A2_IS_SELECT())
        {
            if(A2_IS_ACCESS_READ(address))
              usb_busread(address);
            else
              usb_buswrite(address, value);
        }
#ifdef PLATFORM_A2VGA
        else
        {
            if (A2_IS_RESET(value))
            {
            	sys_reset();
  #ifdef FUNCTION_PROFILER
                continue; // do not consider the "reset" call when profiling
  #endif
            }
        }
#endif

        LOGGER_LOG(value, address);

        // start time measurement (count-down timer)
        PROFILER_STOP(ProfilerMaxTime);
    }
}

static void DELAYED_COPY_CODE(core0_loop)()
{
    for(;;)
        usb_main();
}

extern uint32_t __ram_delayed_copy_source__[];
extern uint32_t __ram_delayed_copy_start__[];
extern uint32_t __ram_delayed_copy_end__[];

int main()
{
    // Adjust system clock for better dividing into other clocks
    set_sys_clock_khz(CONFIG_SYSCLOCK*1000, true);

    // initialize bus interface
    A2_INIT();

    // start processing bus cycles
    multicore_launch_core1(core1_loop);

    // Finish copying remaining data and code from flash to RAM
    dmacpy32(__ram_delayed_copy_start__, __ram_delayed_copy_end__, __ram_delayed_copy_source__);

    // start the normal processing stuff on core 0
    core0_loop();

    return 0;
}
