// SPDX-License-Identifier: zlib-acknowledgement

GLOBAL u32 global_test_val = 0xdeadbeef;

INTERNAL CONSOLE_CMD_STATUS 
mem_read_cmd(String8Node *remaining_args)
{
  u32 read_to_not_discard = global_test_val;

  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  if (remaining_args != NULL)
  {
    String8Node *address = remaining_args;
    b32 parse_err = 0;
    u32 *address_u32 = (u32 *)PTR_FROM_INT(s8_u32(address->string, &parse_err));
    if (parse_err != 1)
    {
      String8Node *count = address->next;
      if (count != NULL)
      {
        u32 count_u32 = s8_u32(count->string, &parse_err);
        while (count_u32-- > 0)
        {
          console_printf("%08x: ", INT_FROM_PTR(address_u32));
          console_printf("%08x ", *address_u32++);
          console_printf("%08x ", *address_u32++);
          console_printf("%08x ", *address_u32++);
          console_printf("%08x\n", *address_u32++);
        }

        result = CONSOLE_CMD_STATUS_SUCCEEDED;
      }
      else
      {
        console_printf("Count not specified\n");
      }
    }
    else
    {
      console_printf("Invalid address passed\n");
    }
  }
  else
  {
    console_printf("No address passed\n");
  }

  return result;
}

// mem write 0x1234 1 0x22

INTERNAL CONSOLE_CMD_STATUS 
mem_write_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  if (remaining_args != NULL)
  {
    String8Node *address = remaining_args;
    b32 parse_err = 0;
    u32 *address_u32 = (u32 *)PTR_FROM_INT(s8_u32(address->string, &parse_err));
    if (parse_err != 1)
    {
      String8Node *data_size = address->next;
      if (data_size != NULL)
      {
        u32 data_size_u32 = s8_u32(data_size->string, &parse_err);

        String8Node *data = data_size->next;
        if (data != NULL)
        {
          u32 data_u32 = s8_u32(data->string, &parse_err);


          result = CONSOLE_CMD_STATUS_SUCCEEDED;
        }
        else
        {
          console_printf("Data not specified\n");
        }
      }
      else
      {
        console_printf("Data size not specified\n");
      }
    }
    else
    {
      console_printf("Invalid address passed\n");
    }
  }
  else
  {
    console_printf("No address passed\n");
  }

  return result;
}



// there is a lot of 'roll-it-yourself' in embedded as every hardware is different
// if wanting to save serial monitor output, best to use Putty on Ubuntu (all could pipe with stty?)

INTERNAL void
mem_add_console_cmds(void)
{
  ConsoleCmdSystem *mem_system = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmdSystem);
  mem_system->name = s8_lit("mem");

  ConsoleCmd *read_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  read_cmd->name = s8_lit("read");
  read_cmd->help = s8_lit("Prints read");
  read_cmd->func = mem_read_cmd;
  SLL_QUEUE_PUSH(mem_system->first, mem_system->last, read_cmd);

  SLL_QUEUE_PUSH(global_console.first, global_console.last, mem_system);
}
