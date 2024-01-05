#pragma once

void memcpy32(void *dst, void *src, uint32_t size);
void dmacpy32(void *start, void *end, void *source);

void __noinline memset32(void *dst, uint8_t val, uint32_t size);

#if 1
#define DELAYED_COPY_CODE(n) __noinline __attribute__((section(".delayed_code."))) n
#else
#define DELAYED_COPY_CODE(n) __noinline __time_critical_func(n)
#endif

#if 1
#define DELAYED_COPY_DATA(n) __attribute__((section(".delayed_data."))) n
#else
#define DELAYED_COPY_DATA(n) n
#endif
