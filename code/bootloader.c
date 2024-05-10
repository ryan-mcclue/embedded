// SPDX-License-Identifier: zlib-acknowledgement
// definition of Flash is to erase in blocks, so is faster than EEPROM
// Flash granularity blocks erased (slow), pages read/written (fast)
// However, to avoid level wearing by erasing block when just want to update page,
// will mark a page as invalid, and write to a new page (handled by Flash Translation Layer)

// STM32 sectors are blocks. they are non-uniform

// NOTE(Ryan): ISP (in-system programming) means external programmer
// IAP (in-application) means ability to update itself without external hardware 

// Normally, firmware is not PIC
// So, the firmware must be flashed at the origin address in linker script, otherwise global variable accesses will fail (why good for dynamic libraries in OS)
// With PIC, we have a GOT (global offset table) that will patch in offsets
// (also have PLT (procedure linkage table))
// So, PIC necessary for having updatable bootloader, i.e. can run from any bank
