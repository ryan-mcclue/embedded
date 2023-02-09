// SPDX-License-Identifier: zlib-acknowledgement
/*
 * Arduino not suitable for industry (overpriced, library licensing issues, only printf debugging)  
 * PCB then PCBA (components attached)
 * On unknown schematic, internet search for chips with most connections
 * Take note of title block (typically on bottom right) on schematic. 
 * First page will typically be a blow-out/legend for subsequent pages. 
 * Useful for looking into electrical diagrams of board components, e.g. mcu pins that are pull-up (will read 1 by default), peripherals (leds to resistors, etc.), st-link, audio, etc.
 * IMPORTANT: Pull-up and pull-down pins may be off to the side in a schematic 
 * ST-LINK is itself an STM32 mcu
 * How does ESD affect the chip?
 * Distinctions between RAM, SRAM, DRAM, etc.
 * Is EEPROM the same as flash? (EEPROM write bytes more power, flash sector)
 * Always check the ERRATA for your mcu/cpu/peripherals to verify source of bug.
 * 'Phil's Lab' for useful DSP software videos. http://www.dspguide.com/pdfbook.htm for text tutorials
 * FPU unit has floating point instructions and DSP instructions?
 *
 * Using these hardware tools to debug are important so as to give EE sufficient information (we are part of an interdisciplinary team)
 * Logic analyser useful for communication protocols that have many digital signals and shows how they work together?
 * Oscilloscope useful for visualising signal?
 * Dev boards typically have jumpers for voltmeter
 *
 * Error handling strategy important upfront
 * Long-term sensors --> graceful degradation
 * Medical applicances --> fail immediately
 * Having an error logging system is essential!
 * IMPORTANT: THESE ARE FEATURES THAT ARE PLANNED UP-FRONT FOR EMBEDDED (DIFFERING TO DESKTOP DEVELOPMENT)
 *
 * Note down performance characteristics of component in code comments when writing drivers, e.g. operating voltage, temperature, etc.
 * (looking at similar open source drivers to see how they configured bus is useful)
 *
   what happens before main() is more interesting for embedded, as it various across each MCU.
   cortex-m defines the placement of vector tables at address 0x04 etc.
   (better to use ARM assembly syntax as common across driver files)
   in startup file will define interrupt handlers as [WEAK] so as to provide a fallback definition
   Best to use assembly for startup code as if using C, compiler optimisations might auto-vectorise which may use FPU registers which may not be initialised yet 
   However, what would be common is that .data are copied to RAM and .bss initialised to 0
   (this becomes more involved with C++ class initialisers)

   DRIVER FILES FOR DISPLAYS (ADAFRUIT)? BE AWARE OF INSPECTING LICENSING

  TODO: Debugging and flashing options (matej)
Flashing:
  1. STM32CubeProgrammer gui --> SWO output? can inspect memory on board
  2. STLINK terminal (st-flash)
  3. JlinkExe
Debugging:
  1. STLINK terminal (st-util)  
  2. JLinkGDBServer

  Unit testing flash: http://www.electronvector.com/blog/unit-testing-with-flash-eeprom 

  In general, unit testing for embedded is typically done natively for simplicity and just testing algorithms
  SPI flash erase byte is 0xff? Can only set by sectors?
 */
