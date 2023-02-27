// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): Is it better to use u8 for indexes as can fit more into cache lines?

typedef struct DIOInput DIOInput;
struct DIOInput
{
  String8 name;
  u32 invert;
  GPIO_TypeDef *gpio_port;
  u32 pin;
};

typedef struct DIOOutput DIOOutput;
struct DIOOutput
{
  String8 name;
  u32 invert;
  GPIO_TypeDef *gpio_port;
  u32 pin;
};

typedef struct DIOs DIOs;
struct DIOs
{
  DIOInput *inputs;
  u32 count_inputs;

  DIOOutput *outputs;
  u32 count_outputs;

  u32 max_num;
};

GLOBAL DIOs global_dios;


INTERNAL u32
dio_add_input(String8 name, GPIO_InitTypeDef *gpio_init, GPIO_TypeDef *gpio_port, u32 invert)
{
  u32 result = 0;

  if (global_dios.count_inputs >= global_dios.max_num)
  {
    LOG_WARNING("Not adding new dio input as reached maximum number\n");
  }
  else
  {
    DIOInput *input = &global_dios.inputs[global_dios.count_inputs];

    input->name = name;
    input->invert = invert;
    input->pin = gpio_init->Pin;
    input->gpio_port = gpio_port;

    HAL_GPIO_Init(gpio_port, gpio_init);

    result = global_dios.count_inputs++;
  }

  return result;
}

INTERNAL u32
dio_add_output(String8 name, GPIO_InitTypeDef *gpio_init, GPIO_TypeDef *gpio_port, u32 invert)
{
  u32 result = 0;

  if (global_dios.count_outputs >= global_dios.max_num)
  {
    LOG_WARNING("Not adding new dio output as reached maximum number\n");
  }
  else
  {
    DIOOutput *output = &global_dios.outputs[global_dios.count_outputs];

    output->name = name;
    output->invert = invert;
    output->pin = gpio_init->Pin;
    output->gpio_port = gpio_port;

    HAL_GPIO_Init(gpio_port, gpio_init);

    result = global_dios.count_outputs++;
  }

  return result;
}

INTERNAL u32
dio_input_get(u32 index)
{
  u32 result = 0;
  if (index < global_dios.count_inputs)
  {
    DIOInput *input = &global_dios.inputs[index];

    result = (u32)HAL_GPIO_ReadPin(input->gpio_port, (u16)input->pin) ^ input->invert;
  }

  return result;
}

INTERNAL u32
dio_output_get(u32 index)
{
  u32 result = 0;
  if (index < global_dios.count_outputs)
  {
    DIOOutput *output = &global_dios.outputs[index];

    result = (u32)HAL_GPIO_ReadPin(output->gpio_port, (u16)output->pin) ^ output->invert;
  }

  return result;
}

INTERNAL void
dio_output_set(u32 index, u32 value)
{
  if (index < global_dios.count_outputs)
  {
    DIOOutput *output = &global_dios.outputs[index];

    HAL_GPIO_WritePin(output->gpio_port, (u16)output->pin, value ^ output->invert);
  }
}

INTERNAL CONSOLE_CMD_STATUS
dio_status_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  console_printf("Inputs:\n");
  for (u32 input_i = 0; input_i < global_dios.count_inputs; input_i += 1)
  {
    console_printf("  %2lu: %.*s = %ld\n", input_i, s8_varg(global_dios.inputs[input_i].name), dio_input_get(input_i));
  }

  console_printf("Outputs:\n");
  for (u32 output_i = 0; output_i < global_dios.count_outputs; output_i += 1)
  {
    console_printf("  %2lu: %.*s = %ld\n", output_i, s8_varg(global_dios.outputs[output_i].name), dio_output_get(output_i));
  }

  result = CONSOLE_CMD_STATUS_SUCCEEDED;

  return result;
}

INTERNAL CONSOLE_CMD_STATUS 
dio_get_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  if (remaining_args != NULL)
  {
    String8 dio_name = remaining_args->string; 

    for (u32 input_i = 0; input_i < global_dios.count_inputs; input_i += 1)
    {
      DIOInput input = global_dios.inputs[input_i];
      
      if (s8_match(input.name, dio_name, S8_MATCH_FLAG_CASE_INSENSITIVE))
      {
        console_printf("%.*s = %ld\n", s8_varg(input.name), dio_input_get(input_i));
        return CONSOLE_CMD_STATUS_SUCCEEDED;
      }
    }

    for (u32 output_i = 0; output_i < global_dios.count_outputs; output_i += 1)
    {
      DIOOutput output = global_dios.outputs[output_i];
      
      if (s8_match(output.name, dio_name, S8_MATCH_FLAG_CASE_INSENSITIVE))
      {
        console_printf("%.*s = %ld\n", s8_varg(output.name), dio_output_get(output_i));
        return CONSOLE_CMD_STATUS_SUCCEEDED;
      }
    }

    console_printf("Unknown dio name %.*s\n", s8_varg(dio_name));
  }
  else
  {
    console_printf("No dio name passed\n");
  }

  return result;
}

INTERNAL CONSOLE_CMD_STATUS 
dio_set_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  if (remaining_args != NULL)
  {
    String8 name = remaining_args->string; 
    u32 index = MAX_U32;

    for (u32 output_i = 0; output_i < global_dios.count_outputs; output_i += 1)
    {
      DIOOutput output = global_dios.outputs[output_i];
      
      if (s8_match(output.name, name, S8_MATCH_FLAG_CASE_INSENSITIVE))
      {
        index = output_i;
        break;
      }
    }

    if (index != MAX_U32)
    {
      String8Node *output_val = remaining_args->next; 
      if (output_val != NULL)
      {
        u32 output_u32 = (u32)(output_val->string.str[0] - '0');
        if (output_u32 == 0 || output_u32 == 1)
        {
          dio_output_set(index, output_u32);
          result = CONSOLE_CMD_STATUS_SUCCEEDED;
        }
        else
        {
          console_printf("Invalid output value passed '%.*s'\n", s8_varg(output_val->string));
        }
      }
      else
      {
        console_printf("No output value passed\n");
      }
    }
    else
    {
      console_printf("Unknown dio name %.*s\n", s8_varg(name));
    }
  }
  else
  {
    console_printf("No dio output name passed\n");
  }

  return result;
}

INTERNAL void
dio_add_console_cmds(void)
{
  ConsoleCmdSystem *dio_system = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmdSystem);
  dio_system->name = s8_lit("dio");

  ConsoleCmd *status_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  status_cmd->name = s8_lit("status");
  status_cmd->help = s8_lit("Prints status");
  status_cmd->func = dio_status_cmd;
  SLL_QUEUE_PUSH(dio_system->first, dio_system->last, status_cmd);

  ConsoleCmd *get_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  get_cmd->name = s8_lit("get");
  get_cmd->help = s8_lit("Return dio information");
  get_cmd->func = dio_get_cmd;
  SLL_QUEUE_PUSH(dio_system->first, dio_system->last, get_cmd);

  ConsoleCmd *set_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  set_cmd->name = s8_lit("set");
  set_cmd->help = s8_lit("Set dio information");
  set_cmd->func = dio_set_cmd;
  SLL_QUEUE_PUSH(dio_system->first, dio_system->last, set_cmd);

  SLL_QUEUE_PUSH(global_console.first, global_console.last, dio_system);
}

INTERNAL void
dio_init(MemArena *perm_arena, u32 max_num)
{
  global_dios.inputs = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, DIOInput, max_num);
  global_dios.outputs = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, DIOOutput, max_num);
  global_dios.max_num = max_num;

  dio_add_console_cmds();
}
