// SPDX-License-Identifier: zlib-acknowledgement

// BOOT1 pin for talking to on-board bootloader?
// this is set in option bits
// perhaps set these option bits for our own bootloader update?

// 2048K flash

// definition of Flash is to erase in blocks, so is faster than EEPROM
// Flash granularity blocks erased (slow), pages read/written (fast)
// However, to avoid level wearing by erasing block when just want to update page,
// will mark a page as invalid, and write to a new page (handled by Flash Translation Layer)

// STM32 sectors are blocks. they are non-uniform

// TODO: https://techblog.paalijarvi.fi/

// NOTE(Ryan): ISP (in-system programming) means external programmer
// IAP (in-application) means ability to update itself without external hardware 

// TODO(Ryan): Include defines in linker scripts

//  1. Once decided on bootloader size, change linker script flash to this (from a duplicated project setup)
//    (will be a block size)
//  2. Change application Flash origin and size in linker script
//     - Also update VECT_TAB_OFFSET appropriately
//  3. Debugger (disable redownloading, only load symbols of multiple binaries) 

// https://github.com/viktorvano/STM32-Bootloader

#define APP1_START (0x08005000)			//Origin + Bootloader size (20kB)
#define APP2_START (0x0800A800)			//Origin + Bootloader size (20kB) + App1 Bank (22kB)
#define FLASH_BANK_SIZE (0X5800)		//22kB
#define FLASH_PAGE_SIZE_USER (0x400)	//1kB

typedef struct
{
    uint32_t		stack_addr;     // Stack Pointer
    application_t*	func_p;        // Program Counter
} JumpStruct;

void jumpToApp(const uint32_t address)
{
	const JumpStruct* vector_p = (JumpStruct*)address;

  // TODO(Ryan): This is because we have initialised clocks/peripherals to facilitate booloader copying 
	deinitEverything();

	/* let's do The Jump! */
    /* Jump, used asm to avoid stack optimization */
    asm("msr msp, %0; bx %1;" : : "r"(vector_p->stack_addr), "r"(vector_p->func_p));
}

void deinitEverything()
{
	//-- reset peripherals to guarantee flawless start of user application
	HAL_GPIO_DeInit(LED_GPIO_Port, LED_Pin);
	HAL_GPIO_DeInit(USB_ENABLE_GPIO_Port, USB_ENABLE_Pin);
	USBD_DeInit(&hUsbDeviceFS);
	  __HAL_RCC_GPIOC_CLK_DISABLE();
	  __HAL_RCC_GPIOD_CLK_DISABLE();
	  __HAL_RCC_GPIOB_CLK_DISABLE();
	  __HAL_RCC_GPIOA_CLK_DISABLE();
	HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
}


#define USER_APP_LOCATION (0x80200000 + 4) // +4 so get vector table not stack pointer

INTERNAL void
jump_to_addr(u32 *addr)
{
  void (*func)(void) = (void *)addr; 
  func();
}

INTERNAL void
bootloader_main(void)
{
  while (1)
  {

  }
}

/*
 * Header (64 bytes; make sizes of header divisible by 4):
 *   8 bytes (sp, vector table)
 *   8 bytes (magic)
 *   12 bytes (device name)
 *   image version
 *   image date
 *   data length (after header; filled in by tool?)
 *   crc32_data_is_valid (says if crc32_data is valid to be used)
 *   crc32_data
 *   crc32_header_is_valid
 *   crc32_header (from bytes 0-60)
 *
 */
// Offset 0 + 8
const uint32_t gcau32ImageStartMagic[2] __attribute__ ((section (“.image_header_body”))) = {0x461C0000, 0x12345678 };

// Offset 8 + 8 = 16
const uint8_t gcau8DeviceName[12] __attribute__ ((section (“.image_header_body”))) =
{‘N’, ‘u’, ‘c’, ‘l’, ‘e’, ‘o’, ‘L’, ‘4’, ‘3’, ‘2’, ‘K’, ‘C’ };

// Offset 16 + 12 = 28
const uint8_t gcau8ImageVersion[8] __attribute__ ((section (“.image_header_body”))) =
{‘v’, ‘.’, ‘1’, ‘.’, ‘2’, ‘.’, ‘7’, 0};

// Offset 28 + 8 = 36
const uint8_t gcau8ImageDate[8] __attribute__ ((section (“.image_header_body”))) =
{‘2’, ‘0’, ‘2’, ‘1’, ‘0’, ‘8’, ‘1’, ‘7’};

// Offset 36 + 8 = 44
const uint32_t gcu32AfterHeaderDataLength __attribute__ ((section (“.image_header_body”))) = 0;

// Offset 44 + 4 = 48
const uint32_t gcu32AfterHeaderDataCrcValid __attribute__ ((section (“.image_header_body”))) = 0;

// Offset 48 + 4 = 52
const uint32_t gcu32AfterHeaderDataCrc32 __attribute__ ((section (“.image_header_body”))) = 0;

// Offset 52 + 4 = 56
const uint32_t gcu32HeaderCrcValid __attribute__ ((section (“.image_header_body”))) = 0;

// Offset 56 + 4 = 60
const uint32_t gcu32HeaderCrc32 __attribute__ ((section (“.image_header_body”))) = 0;

// Offset 60 + 4 = 64


The executable runs through the binary, it does the following:

    1. Checks the actual size of the data after header, puts it in place
    2. Calculates the crc32 of the data after header
    3. Puts in place flag that crc for data after header is correct, also puts in place the crc32 for data after header
    4. Puts in place the flag that header crc is correct
    5. Now that everything else is in place, calculates the crc32 of the header data and places the checksum at the last 4 bytes of the header.
