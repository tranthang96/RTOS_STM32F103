#include "iwdg.h"

#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/common/iwdg_common_all.h>
void iwd_init(void)
{
  iwdg_reset();
  iwdg_set_period_ms(5000);   // watchdog 32s
  iwdg_start();
}