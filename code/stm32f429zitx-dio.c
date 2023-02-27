// SPDX-License-Identifier: zlib-acknowledgement

typedef struct DIOInput DIOInput;
struct DIOInput
{
  String8 name;
  b32 invert;
};

typedef struct DIOs DIOs;
struct DIOs
{
  MemArena *perm_arena;

  u32 max_num;

  struct inputs
  {
    DIOInput *first;
    DIOInput *last;
    u32 count;
  }
  struct outputs
  {
    DIOOutput *first;
    DIOOutput *last;
    u32 count;
  }
};

GLOBAL DIOs global_dios;

INTERNAL void
dio_init(MemArena *perm_arena, u32 max_num)
{
  global_dios.perm_arena = perm_arena; 
  global_dios.max_num = max_num;
}

INTERNAL void
dio_add_input(String8 name)
{
  if (global_dios.inputs.count >= global_dios.max_num)
  {
    LOG_WARN("Not adding new dio input as reached maximum number\n");
  }
  else
  {
    DIOInput *input = MEM_ARENA_PUSH_STRUCT_ZERO(global_dios.perm_arena, DIOInput);

    input->name = name;

    // gpio stuff
    GPIO_InitTypeDef gpio_init = ZERO_STRUCT; 
    gpio_init.Pin = pin;
    // NOTE(Ryan): Almost always PP as want to be able to set 0 and 1
    gpio_init.Mode = GPIO_MODE_AF_PP; 
    // NOTE(Ryan): Really only necessary if reading 
    gpio_init.Pull = GPIO_PULLUP; 
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = uart_params->af;
    HAL_GPIO_Init(base, &gpio_init);

    SLL_QUEUE_PUSH(global_dios.inputs.first, global_dios.inputs.last, input);
  }
}


