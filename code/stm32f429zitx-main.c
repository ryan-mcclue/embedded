//#include "main.h"

// IMPORTANT(Ryan): Put these in every file
// Strange circular includes in st drivers otherwise
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
// NOTE(Ryan): Remove #defines to enable peripherals when required
//#include "stm32f4xx_hal_conf.h"

#include "base-inc.h"

#include "boot.h"

// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

#if defined(TEST_BUILD)
int testable_main(void)
#else
int main(void)
#endif
{
  HAL_Init();

  SystemClock_Config();

  //MemArena *permanent_arena = mem_arena_allocate(KB(32));
  // initialise_global_temp_mem_arenas(KB(32));

  // MemArenaTemp temp_arena = mem_arena_temp_get(NULL, 0);

  // MX_GPIO_Init();
  while (FOREVER)
  {

  }

  return 0;
}
