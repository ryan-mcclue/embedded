# Final Project (for employer)
YOU WANT TO TALK ABOUT THIS IN AN INTERVIEW
TODO: using an IMU to add tilt/interactivity responses, i.e. transform interface to component
TODO: look at classmates project github   
TODO: component selection using digikey, mouser etc. (if in car require wide temperature ranges, if outerspace require radiation hardening)

TODO: when at company, ask how their software development process aligns with waterfall/agile stages

unfortunately no strict commonality using an IDE or compiler make system with embedded.
lots of projects different

step-by-step debugger only possible if chip exposes SWD.
so no feather boards?

linker and map file related?

open-source software with embedded, e.g. openOCD ICD

(a) Use a Cortex-M processor
(b) Have a button that causes an interrupt
(c) Use at least three peripherals such as ADC, DAC, PWM LED, Smart LED, LCD,
sensor, BLE
(d) Have serial port output
(e) Implement an algorithmic piece that makes the system interesting
(f) Implement a state machine

Bonus points are available by including one of these (include graphs):
● An analysis of the power used in the system (expected vs actual)
● Implementation of firmware update with a description in the report of how it works
● A description of profiling the system to make it run faster

(a) Video of the system working as intended (link to mp4 or youtube)
(b) Write up of the system (PDF or Google docs report).
(c) Link to the code

The written part of the assignment is an introduction to the system for another programmer. The
write up should include:
● Application description
● Hardware description
● Software description
○ Describe the code in general
○ Describe the parts you wrote in some detail (maybe 3-5 sentences per module)
○ Describe code you re-used from other sources, including the licenses for those
● Diagram(s) of the architecture
● Build instructions
○ How to build the system (including the toolchain(s))
■ Hardware
■ Software
○ How you debugged and tested the system
● Future
How long would the device last on a battery? What could you do to make it last longer?
○ What would be needed to get this project ready for production?
○ How would you extend this project to do something more? Are there other
features you’d like? How would you go about adding them?

only compile gcc from scratch if wanting to link to specific version of libc on embedded linux

# Week 1
 ## Differentiation
 embedded developers only really concerned with active components not passive?

 Look at schematics to get overview of peripherals on board MCU talks with
 Also, schematic review will be done at some stage during development, so what to be able to 
 talk with EE about say putting more test points on (could just be copper pads (for oscilloscope could solder on a brass loop)), 
 verifying MCU/peripherals are similar to what was used on dev-kit,
 ask about input ranges like what voltage ranges do I expect for ADC?
 Also, can discuss things that might be inexpensive in hardware, but expensive in software

 Even without RTOS, still have a timer subsystem and separation of tasks, which is backbone of RTOS
 
 Feedback on PCB design could be using I2C over SPI in relation to number of wires?

 TODO(Ryan): How does manufacturing software work, e.g. software to coordinate flashing of 1000s of boards

 Fundamentally cool to create devices that interact with world and not just screens 

 Embedded system special-purpose (generally systems are resource constrained for manufacturing cheapness, or less power usage). 
 More restrictions/constraints in software (deterministic, real time), 
 hardware (speed, code, ram), debugging (limited hardware breakpoint timing errors), 
 mass deployment/maintanence

 Learning a new MCU is like a new programming language
 DSP used to be discrete chip, now MCU functinality

 Embedded systems is a team sport of EE and software. So, most come from either background.
 Will always be lacking some knowledge, but need basics of EE in my case
 Could be bug in software or hardware (power supply failing, failed to get cpu errata).
 Therefore, most embedded systems difficult as you often only know half
 Embedded systems interact with our world, not just our screens

 IMPORTANT(Ryan): In addition to being accessed elsewhere (interrupt handler, thread), 
 volatile also indicates accessing something elsewhere (e.g. memory-mapped)
 * schematic reading (larger component first. mcu name, family, memory from name)
 * if having to register-work, use struct cast
 * bugs not just in domain of software. however, best to assume software, then build a case for hardware, e.g. errata, solder glob, psu failing etc. 
   (necesity of HIL to reveal hardware issues?)
   (in fact, any free time should be spent on HIL tests?)
   (TODO: how exactly to write tests to verify hardware is functional)
   Issue with embedded testing is that there is so many ways to test, e.g. simulator, unit, hardware
   debugging thoughts -> how to isolate system first -> has power? -> timing correct? 
 * many embedded systems designed less than optimally, as often have to wait for hardware to be ready, resulting in software quality time getting squeezed
   (e.g. may have to implement software workarounds for things that should be in hardware)

   often have 2 hardware breakpoint units (called "watchpoint units" in ARM's documentation)
   software breakpoint requires modification of code to insert breakpoint instruction

   Have code cross compile for M0+ and M4, as supporting product line, not just one product
   Using multiple compilers when writing different libraries, or even a library/driver may be written for one

  logiic analyser for communication packets
  mutlimeter continuity
  how long function takes --> set GPIO line high when in function --> time signal in oscilloscope (so oscilloscope often used to verify timing)
  IMPORTANT: could take several lines together and send out on a DAC to combine into a single signal for easier viewing
  (so, some soldering might be required when connecting wire to IO line on a PCB board, i.e. not demo board)
  IMPORTANT: major factor in embedded is having ability to test things easily
  so, IO line if want to reduce timing overhead
  IMPORTANT: want MCU that exposes JTAG/SWD pins for debugger  

  component selection is capabilities and then what environment for these components

  Design more pre-planning for embedded. 
(notably hardware block diagrams. 
 so, important to know what protocols used for what common peripherals)
  This is because hardware is involved 
  However, diagrams are just for your own understanding 

  Is more waterfall (in reality a spiral waterfall, i.e ability to go back) as we have to deal with hardware, 
  and can't magically get new boards every 2 weeks like in agile.
  (so, maybe agile for software, but waterfall for the project as a whole?)

  dev prototype is basically stm32 discovery board and breadboard sensors
  actual product will progress from a layout to a gerber file to pcb. 
  once is this state, that is when alpha/beta etc. releases are done  

  Error handling strategy important upfront
  Long-term sensors --> graceful degradation
  Medical applicances --> fail immediately
  Having an error logging system is essential!
  IMPORTANT: THESE ARE FEATURES THAT ARE PLANNED UP-FRONT FOR EMBEDDED (DIFFERING TO DESKTOP DEVELOPMENT)

  Example diagram: 
  https://lucid.app/lucidchart/4877b417-3928-4946-93e2-d6ea91f1451f/edit?beaconFlowId=D87AD983CE11B92D&invitationId=inv_ef9f17ee-abfa-42e3-9069-208d3af34b56&page=0_0#

  More frequent use of state machines? moorly vs mealy state machines?

  Difference between shipping a few devices and a million devices

  PSoC is type of MCU made by Cypress where some peripherals can be programmable from software like in a FPGA? 

  Read book than watch lecture


 ## Schematics
 Schematic first. Larger component, generally more important

# Week 2
 ## Testing
  During dev kit phase, creating a debugging toolchain and 
  testing framework (more tests for more 'riskier' parts) is essential
  Have standard unit/integration tests and 
   POST (power-on-self-tests) which run every time on board power-up
  (probably command console also)
  TODO: POSTS tests like checking battery level, RAM R/W, CRC check? 



 Circuit simulators: https://www.falstad.com/circuit/circuitjs.html
 Button debouncing: https://hackaday.com/2015/12/09/embed-with-elliot-debounce-your-noisy-buttons-part-i/
 Useful serial oscilloscope tool, windows: https://x-io.co.uk/serial-oscilloscope/  
 plantuml.com for code generating diagrams

  code: led triggered by button with interrupt and debouncing (with timer or hardware debouncer?)
  want to debounce say no more than 10Hz to achieve an approx. 16ms user response?
 
  interrupts can be disabled (at this time all further ISRs are pending and resolved based on priority) when in ISR and reenabled on exit.
  
  displays are commonly used in embedded systems that are not battery operated as consume a lot of power


  If something affecting system timing too much perhaps only run in the sleep duty cycle 
  (which is often 90% in battery powered devices) 
  so as to not affect system timing


 Arduino not suitable for industry (overpriced, library licensing issues, only printf debugging)  
 PCB then PCBA (components attached)
 On unknown schematic, internet search for chips with most connections
 Take note of title block (typically on bottom right) on schematic. 
 First page will typically be a blow-out/legend for subsequent pages. 
 Useful for looking into electrical diagrams of board components, e.g. mcu pins that are pull-up (will read 1 by default), peripherals (leds to resistors, etc.), st-link, audio, etc.
 IMPORTANT: Pull-up and pull-down pins may be off to the side in a schematic 
 ST-LINK is itself an STM32 mcu
 How does ESD affect the chip?
 Is EEPROM the same as flash? (EEPROM write bytes more power, flash sector)
 Always check the ERRATA for your mcu/cpu/peripherals to verify source of bug.

SPI flash erase byte is 0xff? Can only set by sectors?

 Using these hardware tools to debug are important so as to give EE sufficient information (we are part of an interdisciplinary team)
 Dev boards typically have jumpers for voltmeter


 If stuck, looking at similar open source drivers to see how they configured bus is useful
 (adafruit, sparkfun, mbed?)

 what happens before main() is more interesting for embedded, as it various across each MCU.
 cortex-m defines the placement of vector tables at address 0x04 etc.
 (better to use ARM assembly syntax as common across driver files)
 in startup file will define interrupt handlers as [WEAK] so as to provide a fallback definition
 Best to use assembly for startup code as if using C, compiler optimisations might auto-vectorise which may use FPU registers which may not be initialised yet 
 However, what would be common is that .data are copied to RAM and .bss initialised to 0
 (this becomes more involved with C++ class initialisers)

   often interrupts and exceptions used interchangeably
   In ARM, an interrupt is a type of exception (changing normal flow of program)
   Exception number (offset into vector table, i.e. exception handler), priority level,
   synchronous/asynchronous, state (pending, active, inactive)
   Index 0 of vector table is reset value of stack pointer (rest exception handlers) 
   On Cortex-M, 6 exceptions always supported: reset, nmi, hardfault, SVCall, PendSV, SysTick 
   External interrupts start from 16 and are configured via NVIC (specifically registers within it)
   Will have to first enable device to generate the interrupt, then set NVIC accordingly
   The startup file will define the interrupt handlers names to which we should define
   Typically an NVIC interrupt handler will have 'sub interrupts' which we can determine with ITStatus()
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  NVIC_EnableIRQ(USART3_IRQn); // cmsis m4

  while (1)
  {
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
    uint16_t adc_data = ADC_GetConversionValue(ADC1);
    float volts_read = (adc_data / ((1 << 12) - 1)) * 5.0f;
    /* temp sensor reading:
     * volts = (adc_data[1] / ((1 << 12) - 1)) * 3.3f;
     * (NOTE: ADCs typically have their own reference voltage to filter out noise)
     * volts -= 0.76; 
     * float celsius = volts / 0.0025f;
     * celsius += 25; (this may not have to be added?)
     */
  }

  // DMA has streams (where data comes in). Channels feed into streams
  // identify the channel, stream and DMA controller number for desired peripheral 
  // (found in reference manual)
  // identify where inside the peripheral we are actually reading data from (most likely a data register)
  // channel 0, stream 0, DMA2 
  

  
  // NOTE(Ryan): Create a 1 second pulse
  timer_init.TIM_Prescaler = 8399; // 84MHz, so 8400 - 1
  timer_init.TIM_Period = 9999; // divide by 10000, so 10000 - 1 
  // The period would affect the resolution for PWM, the prescaler the frequency of PWM
  
  // IMPORTANT(Ryan): For PWM, we require higher frequency, e.g. 1kHz so it appears silky smooth  
  // We relate this value to the period to obtain duty cycle.
  // This is half period, so a 50/50, 50% duty cycle

  // Say we want an output frequency of 1Hz, we play with prescaler and counter values that fit within the 16 bits provided in relation to the input clock
  // E.g. 42MHz in, divide by 42000 (in range of 16bit) gives 1000Hz. Then count to 1000
  // A formula for calculating this will often be given (update event period)
