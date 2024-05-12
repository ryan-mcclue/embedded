// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

// classic RC circuit exponential discharge, 95% fall time 

  // TODO(Ryan): blink LED when doing a reset

  // TODO(Ryan): Debouncing https://youtu.be/yTsjfXsW25A?t=242
  // Adding capacitor smooths signal, so no bouncing. However, delays signal peak time
  // Adding a schmitt trigger fixes this. However, cheaper to do in software
  // Also, what does capacitor between EN (voltage regulator enable pin) to ground do? (to ground will disable it, but the capacitor part?)

  // GPIO_MODE_EVT_FALLING
  // an event is a software controlled flow control mechanism
  // https://stackoverflow.com/questions/16351431/whats-the-difference-between-an-event-and-an-interrupt-in-arm-cortex

  // low power devices seem just to refresh every 30minutes?
  // 3 low power modes (usage, startup time, activity)
  // (REQUIRE KNOWLEDGE OF CLOCK SOURCES)
  // 1. sleep: CPUCLK shuts off
  //           have wakeup events (could be a button press) that are distinct from interrupts
  //           HAL_SuspendTick() prior to entering sleep mode
  //           HAL_PWR_EnterSLEEPMode();
  //           HAL_ResumeTick();
  //           However, seems more flexible to use an interrupt (WFI) to wakeup, e.g. timer, SPI bus data etc.
  // 2. stop: most CLKS off, i.e. all peripheral clocks off (if using WFI, can only use EXTI)
  //          no need to explicitly suspend systick
  //          however, on resumption will have to system_intialise(), i.e. start clocks again
  // 3. standby: SRAM contents lost, most CLKS/oscillators off 
  //             will restart system on wakeup
  //             WFI must be on certain pin, e.g. WKUP pin
  //             still have power say on 3.3V pin
  //             can check if wakeup by inspecting backup registers
  //             TODO(Ryan): could also wakeup from watchdog or rtc
  
  // startup test could be checking if writing to external SDRAM works

  // LED debugging: (create certain patterns to indicate certain errors)
  // green heartbeat led timer indicates if running and if CPU overloaded
  // red led indicates error, say for initialisation error
  // red and green on if in fault handler
  // blink at certain rate for when writing/reading to flash
  // (could solder a current limiting resistor to an LED so can just 'plug-in-play')
  
  // rtc for times like every 24hours? (used for timestamping data, e.g. logging)
  // a timer is just a basic counter and compare
  // rtc more options and on separate power domain, e.g. drift register, crystal source?
  // more accurate readings as direct clock source not PLL?
  // XTAL are pins for external oscillator (on boards, X labels oscillator)
  // RTC stores date, like day of week etc.
  // for accurate, i.e. mininmal drifting stm32 rtc, want it to be clocked with LSE (as its at 32.768KHz which is a sweet spot?) 
  // the LSI is not as accurate? meaning crystal frequency varies?
  // why want low frequency as compared to say HSE?
  // IMPORTANT: LSE must be enabled via RCC
  // RTC operates in backup domain, so on reset will retain (only if power still to VBAT, not necessarily whole system VCC?)
  // Will require initial setting to actual time
  //
  // Not all discovery boards have this on them, rather only have pads for you to solder your own cystal to
  // RTC can be used to timestamp data
