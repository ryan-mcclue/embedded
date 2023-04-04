// SPDX-License-Identifier: zlib-acknowledgement
// definition of Flash is to erase in blocks, so is faster than EEPROM
  // IMPORTANT(Ryan): Will get corrupted data if try to write to a page that hasn't been erased first
// Flash granularity blocks erased (slow), pages read/written (fast)
// However, to avoid level wearing by erasing block when just want to update page,
// will mark a page as invalid, and write to a new page (handled by Flash Translation Layer)

// STM32 sectors are blocks. they are non-uniform

// dual bank just means more sectors, but of smaller sizes

#define SECTOR_COUNT 8

struct FlashSectorData
{
  u32 size, base;

  void *data;

  b32 locked; // lock bootloader sectors?
};

FlashSectorData global_flash_sectors[SECTOR_COUNT];
void
init_flash_sectors(void)
{
  global_flash_sectors[0].base = 0x08004000;
  global_flash_sectors[0].size = 16384;
  global_flash_sectors[0].data = (void *)global_flash_sectors[0].base;
  global_flash_sectors[0].locked = true;
}

void
erase_sector(u8 sector)
{
  if (!global_flash_sectors[sector].locked)
  {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

    // TODO(Ryan): dependent on what voltage system is operating at (sleeping?)
    FLASH_Erase_Sector(sector, FLASH_VOLTAGE_RANGE_3);
    // HAL_FLASHEx_Erase() for erasing multiple sectors in one call?

    HAL_FLASH_Lock();
  }
}

void
write_sector(u8 sector, u32 offset, u8 *data, u32 size)
{
  if (!global_flash_sectors[sector].locked)
  {
    if (offset + size < global_flash_sectors[sector].size)
    {
      HAL_FLASH_Unlock();
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR);

      u32 base_addr = global_flash_sectors[sector].base + offset;
      for (u32 )
      {
        // TODO(Ryan): Write size in words
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, base + i, data[i]);
      }

      HAL_FLASH_Lock();
    }
  }
}

void
erase_and_write_sector(u8 sector, u8 *data, u32 size)
{

}

#define START_OF_FRAME_DELIM 0x11112222
#define END_OF_FRAME_DELIM 0x22221111

#define HEADER_ID 23 

struct Frame {};

void
uart_send_data(); // will send to buffer byte by byte

void send_frame(Frame *frame)
{
  uart_send_data(frame, sizeof(*Frame));
}
