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

#ifdef FUNCTION_LOGGING
	static __always_inline void LOGGER_LOG(uint32_t value, uint16_t address)
	{
	    if ((LogTrigger==2) && ((LogCounter & 0x4000)==0))
	    {
		LogMemory[LogCounter++] = (value&0x03ff) | ((value<<6)&0xffff0000);
		// stop logging on BRK
		if (address == 0xfffe)
		  LogTrigger = 0;
	    }
	    else
	    if ((LogTrigger==1)&&(address == LOGTRIGGER_STARTADDRESS))
		LogTrigger = 2;
        }
	
	static __always_inline void LOGGER_STOP()
	{
	    LogTrigger = 0;
	}
	
#else
	#define LOGGER_LOG(address, value) {}
	#define LOGGER_STOP() {}
#endif


