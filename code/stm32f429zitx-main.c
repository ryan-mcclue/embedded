// IMPORTANT(Ryan): Remove #defines to enable peripherals when required
//#include "stm32f4xx_hal_conf.h"

#if !defined(TEST_BUILD)
  #include "stm32f4xx_hal_msp.c" 
  #include "stm32f4xx_it.c" 
  #include "syscalls.c" 
  #include "sysmem.c" 
  #include "system_stm32f4xx.c"

  #include "stm32f4xx_hal.c"
  #include "stm32f4xx_hal_cortex.c"
  #include "stm32f4xx_hal_rcc.c"
#else
  // IMPORTANT(Ryan): Put these in every file
  // Strange circular includes in st drivers otherwise
  #include "stm32f4xx.h"
  #include "stm32f4xx_hal.h"
#endif

// TODO(Ryan): Have macro definition like in stb libraries to allow for mocking
#include "base-inc.h"

#include "stm32f429zitx-boot.h"

// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

// TODO(Ryan): Have simple way of running functions in native tests, to allow for native debugging logic
INTERNAL void
parse_commands(MemArena *arena, String8 cmd)
{
  String8 space_split = s8_lit(" ");
  String8List cmd_tokens = s8_split(arena, cmd, &space_split, 1);
}

#if defined(TEST_BUILD)
int testable_main(void)
#else
int main(void)
#endif
{
  HAL_Init();
  SystemClock_Config();

  MemArena *permanent_arena = mem_arena_allocate(KB(32));
  initialise_global_temp_mem_arenas(KB(32));

  TempMemArena temp_arena = temp_mem_arena_get(NULL, 0);

  // systick is 1ms; not spectacular resolution
  // important to recognise possible rollover when doing elapsed time calculations

  while (FOREVER)
  {

  }

  return 0;
}
