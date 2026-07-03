#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool bq_init(void);
bool bq_read_all_cell_mv(float cell_mv[12], uint8_t cell_count);
bool bq_read_faults(uint32_t *flags);

#ifdef __cplusplus
}
#endif
