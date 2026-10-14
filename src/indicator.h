#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
extern void indicator_init();

extern void indicator_show(int timeout_ms, int indicator_size, uint32_t color, int position);

#ifdef __cplusplus
}
#endif