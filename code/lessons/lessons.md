# Week 2
 cortex-m defines the placement of vector tables at address 0x04 etc.
 (better to use ARM assembly syntax as common across driver files)
 in startup file will define interrupt handlers as [WEAK] so as to provide a fallback definition
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

## Week 3
  Documentation:
  TODO: be able to read timing diagrams better (e.g. peripheral driver)

  If see like I2C on schematic, then probably connection to peripheral on board like MEMS sensor
  Also, this might be a separate block on page, indicating connecting to another component, e.g. pull-up resistor for I2C audio
  Schematic specific letters:
    - VDD voltage rail to core MCU (also have separate E5V, U5V, 3V3, 5V, VBAT etc. and AVDD/AGND for analog powered)
    - IDD current consumed by device
    - IOREF (voltage logic level; stacking components to set their logic levels)
    - COMP comparator
    - SB solder bridge (control configuration options; cheaper and less intrusive than a switch)
      JP jumper (when shorted with cap, is considered no)
    - P[A-Z] port
      CN connector (exposes various pins)
    - D diode (power isolation, e.g:
               VDD main power and VBAT is backup
               only want VBAT to feed VDD when necessary
               so, diode prevents VDD possibly charging VBAT)
               (schottky diode for low voltage drop, but some reverse current leakage)
    - U unit (ICs like voltage regulators)
    - X oscillator
    - B button
    - T transistor (macroscopic as easier to interface with other components)
      (npn used as electrons generally greater mobility than holes)
    - L inductor (ceramic ferrite; step-up/down voltage; magnetic)
    - C capacitor (charges and discharges; smooths/stabilises/filters and timing)
      R resistor (plenty of voltage dividers, pull-up/down)
    - TP test point
    - SAI (serial audio interface; serial for audio as less power)
    - MCO (master clock output)
  (schematic often found under 'CAD resources' along with gerber files, BOM etc.)

  ARM Debug Interface Architecture
  DAP (Debug Access Port) bus.
  Possible Debug Port are SW-DP, JTAG-DP, SWJ-DP
  Possible Access Port are MEM-AP (access to CPU, flash etc.), JTAG-AP 
  Debugger handles transactions, e.g. request/ack/data phases etc.
  (attackers connect say a bus pirate two various pins and send signals hoping to get valid response)
  (boards implement different ways to prevent access to DAP)
  SWO is an extension of 2pin SWD interface SWDIO/SWDCK
  ITM (Instrumentation Trace Macrocell) typically connected to SWO
  Specific semihosting interrupt request also handled over SWO
  Programmers by default do not provide power

  TODO: https://jaycarlson.net/microcontrollers (for stats about mcus)
  want to be able to measure current and cycles

  TODO: protocol speeds

  power profiler say at 3.3V than at say 2V to account for when battery nearly discharged? 

  https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/test_and_optimize/optimizing/power_general.html


https://www.segger.com/products/debug-probes/j-link/accessories/adapters/target-supply-adapter/
https://shop.segger.com/debug-trace-probes/debug-probes/adapters/supply-adapter

  names referring to shields:
  ST zio connector is extension of arduino uno. is black raised headers
  ST morpho connector are male holes 

  Common in embedded to overload terms, e.g. STM32 board

  AVR 'fuse' bits, e.g. configuration bits (imagine each bit is a jumper wire)
  Could be considered security or lock bits?
  bootloader lock bits? used to control access of flash from code, i.e. application code from bootloader
  or flash/eeprom from being accessed by external tool, i.e. not from MCU (in this case, only way to clear is to do a chip erase)
  (so can't flash again?)
  Boot pins determine where in flash start?
    
  Program board over SWD (with JTAG) or ISP (in-system programmer, a.k.a in-circuit serial programming)

  IC packaging:
  DIP (double inline package; logic gates), SOP (small outline package; voltage regulator), 
  QFN (quad flat package; mcu) and BGA (ball grid array; cpu)

  L3Harris job in queensland (elizabeth roy linkedin)

  Various 'application note' documents and errata (for mcu and peripheral) (e.g. I2C port might not work under certain conditions)
  (application note for 'efficient coding' when looking at low power)

  Cannot debug without understanding of say I2C

  Under no aspersions that working of dev board reflects entire product design
  TODO: how would program go about for PCBA?
  How do dependency between hardware and software development work for project?
  Understanding how interdisciplinary scheduling works?
  So, software can start before hardware ready as work on dev board?
  Software should be flexible to hardware changes

  As problems not always software:
  Need to be able give hardware/electrical engineer as much information as you can to reproduce bug
  (problem not always ground loop or cosmic ray radiation)  

  oscilloscope:
  see if signal matches datasheet?
  current-draw or battery level are truly analog and don't have digital equivalent
  (look at this if logic analyser signals don't make sense)

  further debug:
  rf interference
  low power modes
  JTAG/SWD
  serial console (performs HIL testing to MCU works with each peripheral successfully before putting in enclosure) 
  (can be used to create a reproducible problem on multiple boards; or on cue for an oscilloscope capture/EE to see)

  map files: https://www.youtube.com/watch?v=XRXLUcbJIxY

  So, when reporting issue:
    * verify signal
    * supply console command
    * mention datasheet info, e.g timing diagram
    * (look for open-source drivers that perform similarly)

  Essentially with embedded, have lots of testing to mitigate issues so TEAM can solve problems
  hardware and software people need to be in it together
  shipping products is a team sport

  Flash erase is 0xFF?
  Pages vs sectors vs buffers (would be slower, i.e. must take into account sector boundaries)?
  EM100Pro-G2 SPI NOR Flash Emulator


## Week 4
Everything in embedded systems is input/output/timers, i.e. hold some line high for a period of time

Although plenty of libraries, like Arduino, mbed no JTAG, just serial.
Also, seriously bloated, e.g. setting duty cycle checks config, etc. when it just needs to set a register value

Hardware semaphores used in MCU to coordinate booting between multiple CPUs, e.g. M4 + M7

When setting a mask, know that at some stage it will probably end up in a register  

When reading through driver code, must have datasheet up to understand why they are doing certain things

## Week 5
Technically everything with an 'if' is a state machine

For a state machine, think about each state:
* action (i.e internal behaviour)
* overlays, e.g. animation, audio
* inputs
* state transitions
Adding a state is cheap

If have to give to QA, encode in a spreadsheet to present as a state table

Have nested state machines
TODO: understand table based state machine
TODO: https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM for RTOS

```
enum STATE
{
  STATE_START,
};

#define STATE_NONE STATE_START (error handling?)

struct StateInfo
{
  STATE current_state;
  char *debug_state_name;

  u32 anim_t;

  // input received, what state will jump to
  // IMPORTANT: inputs in embedded will typically be interrupts that set
  // a variable
  STATE button_pressed_input;

  // actions
  function_pointer(prev_state, ...) action; // this is where can have large switch statement we expect in state machines?
};


StateInfo state_table[] = 
{

};

main
{
  // state_table[current_state][input] ??
  state->action_fp(NULL, ...);
}

enum states {
	START,
	LOOP,
	END,
} state;

enum events {
	START_LOOPING,
	PRINT_HELLO,
	STOP_LOOPING,
};

typedef enum states (*event_handler)(enum states, enum events);

enum states start_looping(enum states state, enum events event) {
	assert(state == START && event == START_LOOPING);
	return LOOP;
}

enum states print_hello(enum states state, enum events event) {
	assert(state == LOOP && event == PRINT_HELLO); printf("Hello World!\n"); return LOOP;
}

enum states stop_looping(enum states state, enum events event) {
	assert(state == LOOP && event == STOP_LOOPING);
	return END;
}

event_handler transitions[STOP_LOOPING+1][END+1] = {
	[START] = { [START_LOOPING] = start_looping, },
	[LOOP] = { [PRINT_HELLO] = print_hello,
	           [STOP_LOOPING] = stop_looping, },
};

void step_state(enum events event) {
	event_handler handler = transitions[event][state];
	if (!handler)
		exit(1);
	state = handler(state, event);
}

int main(void) {
	step_state(START_LOOPING);
	step_state(PRINT_HELLO);
	step_state(PRINT_HELLO);
	step_state(STOP_LOOPING);
	return 0;
}
```

Before entering an interrupt, context is saved

RTOS:
  * scheduler (priority, time slices, preemptive)
  * resource sharing

Embedded:
  * state machines
  * things that are continously monitored (circular buffers)

Many embedded compilers lag behind modern desktop compiler features

Watchdog are timer features?
They are NMI
Time between petting the dog varies
Most debuggers stop the timer on a breakpoint (seems that debugger setup can even disable timers, DMA, etc.)
(will have to stop watchdog during a firmware update)
Place in idle loop

TODO: Charlieplexing outputs e.g. LEDS and matrix inputs e.g. buttons

For most engineering, the date between shipping and actually appearing on shelves could be 4 months.

## Week 6
As MEMS are small, sensitive to temperature (in fact all sensors can be thought of temperature sensors to an extent)
(so, calibration of sensors is possible, however expensive and what to avoid)
IMU/inertial sensors 9DOF: 
  * accelerometer
    Saying measure acceleration technically true, but not helpful?
    90% of the time, accelerometers tell us which way is down
    tell you z (what side facing)
    However, if moving 10%, measure acceleration in particular axis
  * gyro 
    measure force needed to turn in another axis (how fast something is turning?)
    typically for gesture detection (are rotating)
  * magnemoter
    tell you x and y on Earth by saying where Earth's magentic field is pointing, i.e. offset from magnetic poles
    (where pointing)
Most likely digital (nice as no analog signals to carry noise)

IMPORTANT: Recognise situations where MEMS sensors can be trusted, e.g.
barometer to measure altitude only good if moving, as air pressure can change from weather

x, y, z Euler angles around axis are roll (literal rolling), pitch (up hill) and yaw (turning)

accelerometer + gyro + kalmann filter gives Euler angles so can tell orientation

Now to get position:
  * accelerometer: double integration
  * gyro: single integration
However, these sensors to imprecise, e.g. vibrations
These two calculations of position wildly different
Kalman algorithm takes prior knowledge of state (i.e. are way moving, stationary so as to know which sensor more reliable) and these measurements 
So, high quality sensors required
Kalman different implementations for different vehicles, e.g. car, boat, spaceship

Quaternion if four-space math. 4th dimension is not time
Used to remove discontinuities, e.g. 359-0 boundary

Yaw tells about relative turns, but doesn't give what direction facing in world

Heading direction vehicle pointing in reference to magnetic north
Bearing is direction travelling (e.g. drifting a corner, heading and bearing are different)

Seems like cryptography, use well established boffin libraries for IMU

There are data driven applications that don't heavily rely on state machines, e.g.
EEG, seisometer, flight recorder
TODO: Use excel formulas
Think about throughput when recieving data from peripheral
Seems we work with buffer sizes relating to seconds of stored data
ADC Throughput to PC:
  - Does board have enough RAM + Flash (internal and external) + CPU power?
  - (12bits * 512Hz * 2channels) * 1.2protocol-overhead = 1.8kBps 
    looking at UART baud rates, we see that it satisfies (no need for USB)
  We have external flash, so might not always need to be connected to PC
  - (16MB / (((1.8 * 1.1crc-timestamp-overhead) * 0.5compression)) / 1000)) = 16161 seconds of ADC data
    if wanting hours, probably better for say a 32Gb sd-card 
  TODO: seems that whenever storing data from say ADC, use a circular buffer?

In general, CPU should not being copying data from place to place.
Instead, DMA should do this.
CPU should be used to analyse data
IMPORTANT: MCU DMA different to desktop DMA
If many people have encountered a common problem, will typically be an application note, e.g. DMA, DSP, encryption
DMA arbitration like interrupt priorities

AHB bus specific to cortex-m processors?

TODO: Seems that common to sleep at end of superloop and wait for an interrupt to occur?

IMPORTANT: In addition to HAL files, typically have example source code for particular MCU on github from vendor


www.reddit.com/r/embedded/comments/8bx71i/error_handling_in_embedded_c/
https://www.reddit.com/r/embedded/comments/czffv8/what_is_your_preferred_method_for_handling_errors/

```
assert for things never should happen
peripheral errors (common in say bluetooth, etc.) always log and handle with error codes (HOWEVER, FIRST BASIC ERROR HANDLING STRATEGY IS LED BLINKING)

Programmer errors use asserts

Software errors:
  * (on dev): capture state of the device (heap, stacks, registers, the works) and assert
  * (release): error codes

Peripheral errors:
  error codes
  log these sorts of errors as well (its critical to know if the hardware is actually bad) (change dev assert from breakpoint to logging)
  IMPORTANT: logging done whatever is available, e.g. UART console, ethernet, flash etc.

Critical errors:
  stack overflows, hard faults, asserts, watchdogs, it's best to save off what caused the crash so it can be sent as a bug report upon reboot.
  For a device installed in people's homes and connected over the internet a last resort is reboot into a "recovery" image. 
  This image does not do the usual work of the device, instead it reports the fault over the network and then waits for repairs to be sent back over the network.

IMPORTANT(Ryan): Although called 'error', ideally want to handle them so they become part of normal program flow
Distinction between debug and release is actually handling errors as oppose to just asserting on them
typedef enum
{
   // start at 0 as represent number of led blinks
   ERR_CC1101_TXFIFO_EMPTY = 1,
   ERR_FLASH_CORRUPTED,
} ERROR_TYPE;

handle these errors with an error handler that blinks an LED in development
later, actually handle errors until all removed.
Handling error:
  1. Continue onwards
  2. Go to failsafe, i.e. rollback state to known good condition (retry, reboot, wait)
  3. Crash and wait for reboot

while (1)
{
  throw_bone();

  // still use enums for error codes, just have error handling centralised (as more thread/interrupt safe?)
  result = state_machine[state]();

  if (result != OK)
  {
     error_handler(result);
  }
}
```

RAM usage:
  .cinit (globals and static with initialisers)
  .bss (globals and static with on initialisers)
  .heap (growing down)  
  ........... NOTE: RTOS tasks will have their own heaps+stacks
  .stack (growing up)
Embedded avoid malloc()s as fragmentation becomes more of an issue, when dealing with small allocation sizes

non-volatile RAM more like flash, except speeds like RAM (as literal name is a contradiction)
keep variables on waking up from deep sleep
TODO: can mark certain areas of flash as non-volatile?

Going from devboard, e.g. Discovery to PCB:
  * remove ICDI chip 
    (replace with JTAG pins. could use Tag-Connect, i.e. springy pogo pin connector in plated holes or pads instead of headers)
    (flash-bin && play banjo-kazooie.mp3 && sleep 1 && goto start)
  * remove UART-to-Serial chip
    (replace with FTDI cable)
  * remove unused, e.g. ethernet, USB etc.
    (replace with battery? wall wart?)
  * for speed add codec, e.g. audio TLV320AIC
  * smaller surface area, remove jumpers (IDD pin and actual GPIO pins), buttons, sensors, leds, plated holes, etc.
    (add test pins where things can go wrong e.g. ground pins, power rails, communication busses)
    (balancing act between how small product board is. could add an 'elephant board' concept)
  * FCC/EMC for unintentional EM radiation (other certifications)
    (can actually search for product on ffcid.io)

For example projects look at hackaday prize winners?

## Week 7 
Schedule slips, so decide features included in update in the field (even more so as time between sending to factory and in user hands is a few months for consumer products)
Often when turning on a new product have to power cycle (turn on and off) for firmware update

On board bootloader cannot be changed after leaving factory
Code will be communicated to it and written to codespace
If programming fails due to power outage, bootloader can retry
Really only use on-board bootloader if multiple MCUs or bootloader is built into silicon for extremely resource constrained
(multiple MCUs useful for low-power as can have large one mostly dormant until required?)
(or STM MCU and say a BLE MCU?)

We want an custom bootloader:
  * both bootloader and runtime code updated separately
  * code can be validated
  * 2x size of programmed code in flash

Due to IoT, our devices could become botnets or leak user data
So, use CRC (hash is better) to ensure image sent is image recieved 
Also, sign so we know where it came from
Per-company keys is easier, however per device key is more secure
At a minimum, OTA bundle:
  * version
  * hash
  * signature
  * (3x flash size to have a known 'factory' image?)

Now, as scaling to 1million devices; will encounter 1 in a million problems
Therefore, device monitoring:
  * error log
  * heartbeat log (to show alive, e.g. power usage and battery life)
  * diagnostics (what device crashed, what firmware are they running, etc.)

General security:
  * CI
  * static analysers
  * treat all external data as malicious:
    - buffer overflows; generally from malformed user input (prevent stack frame overwrite for code execution)
    (IoT chipset has more attack vectors than traditional embedded, so more careful) 

https://bugprove.com/knowledge-hub/enhancing-device-security-beyond-firmware-encryption/

security, bootloader updates and general device management at start of project?

NRF connect for automating BLE bootloader testing
Also have phone apps for DFU (better to test on phone, )
AWS device farm?

INTERVIEWING:
1. Resume

2. Phone Screens:
  - technical word bingo

3. In Person:
  In interview, as a student can bring modern software practices
  (perhaps a diversity of thought, to get a 360degree view of subject)
  This is the portfolio I have, here is want I can bring

  * Want to know can they work with you
  * Want to know can you do the job
  (know failures)

  Ask you to write pseudo-code solving a problem, e.g. 4-way intersection:
    - ask clarifying details so they know thinking about big picture
    (problem include a timeout for sensors that aren't working)
    (state machine)

  Ask the interviewer:
    - do you like your job?
    - describe typical work day?
    
  situational interviewing

  Respect and appreciate the time taken for the interviewer (spent all day on call)

  1. Creativity
  2. Technical Knowledge
  3. People Skills

## Week 8
Map files, memory layout of system, i.e. RAM and Flash (bootloaders, error traces)
(perhaps linker script more informative for sections though)
Memory map could give indication for memory corruption, e.g overwriting stack or global?
Memory layout in Flash
  * image header (serial number, keys)
  * reset vectors (these could be in flash, i.e could be functions copied over to .ramfuncs in cstartup for speed?)
  * .text
  * .rodata/.consts
  * .cinit
  * ...
  * nv storage? (could also have some storage system, file system etc.)
  * bootloader?

PSRAM (pseudo static RAM)

Optimising Memory:
Map file can be used to see what is taking up RAM or Flash, e.g. string constants
Look at largest consumers first
Could be monolithic library functions that need replacing

Debugging:
* Debugger will contain backtrace information. Compare this with information in map file
* Memory corruption (global buffer overflowing) 
* Tracing; as storing strings memory costly, just address and compare that with map file to see what function

Bootloader first check if new code available, then always check if code about to jump to is valid

TODO: memory has wait states? so, move critical function to RAM with less wait state than Flash? (in fact, RAM is zero wait state?)

IMPORTANT: DMA is part of ARM core. As is AHB bus (which DMA, Flash, SRAM talk to)

Very likely that things change as project progresses, e.g. want AI now, so current part is not fast enough nor has enough flash for machine learning
The part was not a bad choice; it was the best choice at that time; now things have changed (that's what an engineer is, making mistakes and learning from them)
Typically start out with a MCU family in the middle, e.g. don't need M4F with all bells and whistles.
If down the road, need to optimise for power, can go down etc.

Alias Flash to 0x00 so can relocate interrupt table

IoT (all require some form of online device sign up?):
  * BLE -> Phone -> WiFi (portable)
    id 
  * WiFi (stationary)
    SSID, password, AP mode
  * Cell modem (portable constant coverage)
  * Lora (intermittent data; not really for consumer products as configuration esoteric, noise susceptible and particular base stations)
  (ZigBee like BLE and WiFi but slow)

IMPORTANT: heap and stack pointer are next available 

## Week 9
Taylor series useful for approximating functions, e.g. quicker to perform Taylor series expansion of say `e**0.1`

CMSIS is for all cortex processors

CMSIS reference for neural networks, fft, etc.
Q numbers are fixed point numbers

When using ADC, can't just blindly read values.
Must know signal looking at due to Nyquist sampling rate.
Also, require how noisy to use filters 
Then, need to know dynamic range, i.e. how big is signal (bit depth will introduce quantisation errors)
So, higher bit depth can increase dynamic range or decrease quantisation error
A pure signal should use a low bit depth to save on data

Reference voltage for analog sensors and ADCs should be as noise free as possible
(so, using a SPI ADC less noise?)
`V = adc_counts * (3.3 / 255)`

Possible if ADC does not have FIFO, have to read out channel 1 data before channel 2 conversion time otherwise overwritten

Generally, don't recover from a HardFault

## Week 10
http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

Reduce power by reducing:
  * Voltage (component selection, e.g. less components in sensor)
  * Current (less code)
  * Resistance (component selection, e.g. less components in sensor)
  * Time (slowly, sleep)

Component Selection:
Reading datasheet, minimum Vdd less than say 3.3V. 
So, to get less power, use voltage divider? 
Compiling a list of sensor information, can pick which one
(some may have different power modes, e.g. idle, sleep, measuring etc.)

For low power, sleep has to be at forefront
i.e. we don't wait for things to happen
Essentially, sleep for as long and as deep as possible

Application Notes for lower power. GPIO settings.
Light sleep (low milliamp to microamp)
Deep sleep (microamp)
Hibernate (nanoamp)
IMPORTANT: peripherals can be set to idle as well

IMPORTANT: use spreadsheet for power estimation

Low Power Questions (At Start):
1. How big battery can fit and what price?
11mm x 4mm ($5)
Found 40mAh, 3.7V
2. How long must unit work between charging?
At least 24 hours
System can average (0.004 / 24) mA
3. What are estimated pieces of system?
oled (12mA, 0), accelerometer (0.165mA, 0.006mA), battery
on state last (), sleep state last ()
We see that cannot be on all the time, as average current less than what battery can provide
4. Resultant restrictions
Tweak on percentage until average current usage is within bounds
Screen only on 5% of time

Low Power Questions (At End):
1. Different states device can be in?
on, light sleep
2. How long in each state
on 5 seconds every 5 minutes, i.e:
on (5seconds, 0.02), sleep (300seconds, 0.98)
3. How much current in each state
on (12mA), sleep (0.14mA)
4. How long device last on 40mAh battery?
(0.004) / ((0.12 * 0.02) + (0.00014 * 0.98)) hours

DMA allows core to be turned off?
Lower power more concerned with peripherals, rather than actual core

Although multimeter good, doesn't show varying from active to sleep states
Oscilloscope shows varying, however if very low power, resistor becomes issue (burden measurement voltage)
Also, large magnitude changes difficult to see

In general, view what is changing, rather than exact current value

IMPORTANT: JTAG gets Vcc from board, so okay to use with external battery
(as long as Vcc pin from board tied to JTAG pin)

For PPK2, use GPIO triggering when measuring from a battery so as to not create ground loop from serial console

STM32 virtual EEPROM provides a emulation layer a top of Flash, for byte level writing
Could view EEPROM as a key-value store?

Always use internal pull-ups if possible (sometimes not because too weak),
so can disable for lower power

IMPORTANT: for console command groups, have a turn on and turn off command for power analysis

Want test points for any communication bus that can fail
Also want some current test points

So, for low power GPIO, could maintain clock, but turn off internal pull-up (which is a current source)
and perhaps switch to analog mode, i.e. acts as input so not driving anything?

In nanoamp range, things like cleanliness of board matter, e.g. flux 

For monolithic things like PlatformIO, version maintenance of libraries is a nightmare (i.e. packaging problems)

UF2 is USB drag-and-drop bootloader file format

Vin typically connected to a voltage regulator. linear regulator, so don't put too high voltage.
drop-out regulator, so 5V to 3.3V out fine

Vbat backup for RTC?

ASK MENTORS THREAD

## Week 12
All about layers:
transistors -> logic gates -> adders/subtractors/multipliers -> ALU
transistors -> logic gates -> flip-flops/latches -> memory

In semiconductor; 2 areas: chip design with logic gates and physics/chemistry of transistors, lithography

Have clock for:
1. Synchronisation; knowing when data is valid
2. Off chip communication; e.g. UART and CPU both at 5KHz; need to have a rate to compare to

Machine learning a lot of linear algebra, so energy intensive and large die size required

ASIC (cheapest) -> CPU (limited by ISA) -> FPGA (basically anything limited by size)

Chip die-image: cores, shared cache, memory controller
Core die-image: i-cache -> decode unit -> scheduler/branch-predictor (stores previous branch instructions) -> ALU -> load/store unit 
(larger sizes on die more complicated, e.g. floating point unit much larger than ALU)
On a die image, if 'plaid' pattern, indicates regular repeating, e.g. memory

Registers fastest memory that lives in scheduler?

Branch prediction for interleaved execution? If wrong, has to clear state

Generally, performance more tenable for 'high-level programmers' when discussing data centres for machine learning and getting more computation per watt of power

synchronous (more expensive than asynchronous) switching (more expensive than linear) step-down (or buck) regulator
(expensive as gets higher efficiency at lighter loads)
