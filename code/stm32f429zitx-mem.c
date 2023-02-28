// SPDX-License-Identifier: zlib-acknowledgement

INTERNAL CONSOLE_CMD_STATUS 
mem_read_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  if (remaining_args != NULL)
  {
    String8Node *address = remaining_args;
    b32 parse_err = 0;
    u32 *address_u32 = (u32 *)s8_u32(address->string, &parse_err);
    if (parse_err != 1)
    {
      String8Node *count = address->next;
      if (count != NULL)
      {
        u32 count_u32 = s8_u32(count->string, &parse_err);
        while (count_u32-- > 0)
        {
          console_printf("%08x: ", (u32)address_u32);
          console_printf("%08x %08x %08x %08x\n", *address_u32++, *address_u32++, *address_u32++, *address_u32++);
        }
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

INTERNAL void
dio_add_console_cmds(void)
{
  ConsoleCmdSystem *mem_system = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmdSystem);
  mem_system->name = s8_lit("mem");

  ConsoleCmd *read_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  read_cmd->name = s8_lit("read");
  read_cmd->help = s8_lit("Prints read");
  read_cmd->func = mem_read_cmd;
  SLL_QUEUE_PUSH(mem_system->first, mem_system->last, read_cmd);

  ConsoleCmd *write_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  write_cmd->name = s8_lit("write");
  write_cmd->help = s8_lit("Return dio information");
  write_cmd->func = mem_write_cmd;
  SLL_QUEUE_PUSH(mem_system->first, mem_system->last, write_cmd);

  SLL_QUEUE_PUSH(global_console.first, global_console.last, mem_system);
}

INTERNAL void
add_mem_console_commands(void)
{

}
