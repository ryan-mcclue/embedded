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

INTERNAL void
dio_init(MemArena *perm_arena, u32 max_num)
{
  global_dios.inputs = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, DIOInput, max_num);
  global_dios.outputs = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, DIOOutput, max_num);
  global_dios.max_num = max_num;
}

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

#if 0
INTERNAL CONSOLE_CMD_STATUS
dio_status_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

    uint32_t idx;
    
    printf("Inputs:\n");
    for (idx = 0; idx < cfg->num_inputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->inputs[idx].name, dio_get(idx));
    

    printf("Outputs:\n");
    for (idx = 0; idx < cfg->num_outputs; idx++)
        printf("  %2lu: %s = %ld\n", idx, cfg->outputs[idx].name,
               dio_get_out(idx));

    return 0;

  return result;
}

static int32_t cmd_dio_get(int32_t argc, const char** argv)
{
    uint32_t idx;
    struct cmd_arg_val arg_vals[1];

    if (cmd_parse_args(argc-2, argv+2, "s", arg_vals) != 1)
        return MOD_ERR_BAD_CMD;

    for (idx = 0; idx < cfg->num_inputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->inputs[idx].name) == 0)
            break;
    if (idx < cfg->num_inputs) {
        printf("%s = %ld\n", cfg->inputs[idx].name, dio_get(idx));
        return 0;
    }

    for (idx = 0; idx < cfg->num_outputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0)
            break;
    if (idx < cfg->num_outputs) {
        printf("%s %ld\n", cfg->outputs[idx].name, dio_get_out(idx));
        return 0;
    }
    printf("Invalid dio input/output name '%s'\n", arg_vals[0].val.s);
    return MOD_ERR_ARG;
}

/*
 * @brief Console command function for "dio set".
 *
 * @param[in] argc Number of arguments, including "dio".
 * @param[in] argv Argument values, including "dio".
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: dio set <output-name> {0|1}
 */
static int32_t cmd_dio_set(int32_t argc, const char** argv)
{
    uint32_t idx;
    struct cmd_arg_val arg_vals[2];
    uint32_t value;

    if (cmd_parse_args(argc-2, argv+2, "su", arg_vals) != 2)
        return MOD_ERR_BAD_CMD;

    for (idx = 0; idx < cfg->num_outputs; idx++)
        if (strcasecmp(arg_vals[0].val.s, cfg->outputs[idx].name) == 0)
            break;
    if (idx >= cfg->num_outputs) {
        printf("Invalid dio name '%s'\n", arg_vals[0].val.s);
        return MOD_ERR_ARG;
    }

    value = arg_vals[1].val.u;
    if (value != 0 && value != 1) {
        printf("Invalid value '%s'\n", argv[3]);
        return MOD_ERR_ARG;
    }
    return dio_set(idx, value);
}


INTERNAL void
timer_add_console_cmds(void)
{
  ConsoleCmdSystem *timer_system = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmdSystem);
  timer_system->name = s8_lit("timer");

  ConsoleCmd *status_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  status_cmd->name = s8_lit("status");
  status_cmd->help = s8_lit("Prints status");
  status_cmd->func = timer_status_cmd;

  SLL_QUEUE_PUSH(timer_system->first, timer_system->last, status_cmd);
  SLL_QUEUE_PUSH(global_console.first, global_console.last, timer_system);
}
#endif
