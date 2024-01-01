#pragma once

void memcpy32(void *dst, void *src, uint32_t size);
void dmacpy32(void *start, void *end, void *source);

void __noinline memset32(void *dst, uint8_t val, uint32_t size);