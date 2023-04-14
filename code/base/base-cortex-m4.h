// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_CORTEX_M4_H)
#define BASE_CORTEX_M4_H

// ENIAC breakpoint was physically unplugging cable  
// cortexm4 uses FPB (flash patch and breakpoint unit) for hardware breakpoints

// hardware breakpoint literally compares instruction to register (property of silicon)
// software implemented by debugger (patches code, so issues with ROM or crashing leaving in bad state)

bool fpb_set_breakpoint(size_t comp_id, uint32_t instr_addr) {
  if (instr_addr >= 0x20000000) {
    // for revision 1 only breakpoints in code can be installed :/
    return false;
  }
  // make sure the FPB is enabled
  FPB->FP_CTRL |= 0x3;

  const uint32_t replace = (instr_addr & 0x2) == 0 ? 1 : 2;
  const uint32_t fp_comp = (instr_addr & ~0x3) | 0x1 | (replace << 30);
  FPB->FP_COMP[comp_id] = fp_comp;
  return true;
}


// DebugMonitor allows for debugging in field without an actual debugger attached?
// only works when certain bit not set by debugger over SWD

bool debug_monitor_enable(void) {
  volatile uint32_t *dhcsr = (uint32_t*)0xE000EDF0;
  if ((*dhcsr & 0x1) != 0) {
    EXAMPLE_LOG("Halting Debug Enabled - "
                "Can't Enable Monitor Mode Debug!");
    return;
  }

  volatile uint32_t *demcr = (uint32_t*)0xE000EDFC;
  const uint32_t mon_en_bit = 16;
  *demcr |= 1 << mon_en_bit;

  // Priority for DebugMonitor Exception is bits[7:0].
  // We will use the lowest priority so other ISRs can
  // fire while in the DebugMonitor Interrupt
  volatile uint32_t *shpr3 = (uint32_t *)0xE000ED20;
  *shpr3 = 0xff;

  EXAMPLE_LOG("Monitor Mode Debug Enabled!");
  return true;
}

typedef enum {
  kDebugState_None,
  kDebugState_SingleStep,
} eDebugState;

static eDebugState s_user_requested_debug_state = kDebugState_None;

void debug_monitor_handler_c(sContextStateFrame *frame) {
  volatile uint32_t *demcr = (uint32_t *)0xE000EDFC;

  volatile uint32_t *dfsr = (uint32_t *)0xE000ED30;
  const uint32_t dfsr_dwt_evt_bitmask = (1 << 2);
  const uint32_t dfsr_bkpt_evt_bitmask = (1 << 1);
  const uint32_t dfsr_halt_evt_bitmask = (1 << 0);
  const bool is_dwt_dbg_evt = (*dfsr & dfsr_dwt_evt_bitmask);
  const bool is_bkpt_dbg_evt = (*dfsr & dfsr_bkpt_evt_bitmask);
  const bool is_halt_dbg_evt = (*dfsr & dfsr_halt_evt_bitmask);

  EXAMPLE_LOG("DebugMonitor Exception");

  EXAMPLE_LOG("DEMCR: 0x%08x", *demcr);
  EXAMPLE_LOG("DFSR:  0x%08x (bkpt=%d, halt=%d, dwt=%d)", *dfsr,
              (int)is_bkpt_dbg_evt, (int)is_halt_dbg_evt,
              (int)is_dwt_dbg_evt);

  EXAMPLE_LOG("Register Dump");
  EXAMPLE_LOG(" r0  =0x%08x", frame->r0);
  EXAMPLE_LOG(" r1  =0x%08x", frame->r1);
  EXAMPLE_LOG(" r2  =0x%08x", frame->r2);
  EXAMPLE_LOG(" r3  =0x%08x", frame->r3);
  EXAMPLE_LOG(" r12 =0x%08x", frame->r12);
  EXAMPLE_LOG(" lr  =0x%08x", frame->lr);
  EXAMPLE_LOG(" pc  =0x%08x", frame->return_address);
  EXAMPLE_LOG(" xpsr=0x%08x", frame->xpsr);

if (is_dwt_dbg_evt || is_bkpt_dbg_evt ||
      (s_user_requested_debug_state == kDebugState_SingleStep))  {
    EXAMPLE_LOG("Debug Event Detected, Awaiting 'c' or 's'");
    while (1) {
      char c;
      if (!shell_port_getchar(&c)) {
        continue;
      }

      EXAMPLE_LOG("Got char '%c'!\n", c);
      if (c == 'c') {
        s_user_requested_debug_state = kDebugState_None;
        break;
      } else if (c == 's') {
        s_user_requested_debug_state = kDebugState_SingleStep;
        break;
      }
    }
  } else {
    EXAMPLE_LOG("Resuming ...");
  }

const uint32_t demcr_single_step_mask = (1 << 18);

  if (is_bkpt_dbg_evt) {
    const uint16_t instruction = *(uint16_t*)frame->return_address;
    if ((instruction & 0xff00) == 0xbe00) {
      // advance past breakpoint instruction
      frame->return_address += sizeof(instruction);
    } else {
      // It's a FPB generated breakpoint
      // We need to disable the FPB and single-step
      fpb_disable();
      EXAMPLE_LOG("Single-Stepping over FPB at 0x%x", frame->return_address);
    }

    // single-step to the next instruction
    // This will cause a DebugMonitor interrupt to fire
    // once we return from the exception and a single
    // instruction has been executed. The HALTED bit
    // will be set in the DFSR when this happens.
    *demcr |= (demcr_single_step_mask);
    // We have serviced the breakpoint event so clear mask
    *dfsr = dfsr_bkpt_evt_bitmask;
  } else if (is_halt_dbg_evt) {
    // re-enable FPB in case we got here via single-step
    // for a BKPT debug event
    fpb_enable();

    if (s_debug_state != kDebugState_SingleStep) {
      *demcr &= ~(demcr_single_step_mask);
    }

    // We have serviced the single step event so clear mask
    *dfsr = dfsr_halt_evt_bitmask;
  }

}


__attribute__((naked))
void DebugMon_Handler(void) {
  __asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq r0, msp \n"
      "mrsne r0, psp \n"
      "b debug_monitor_handler_c \n");
}



// ARM debugging: halt debugging, tracing (DWT, ITM, ETM (jtrace)) and debugmonitor


// TODO(Ryan): Segger RTT https://www.youtube.com/watch?v=vl4km1TmLkg

#define INIT_CYCLE_COUNTER() \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk

#define ENABLE_CYCLE_COUNTER() \
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk

#define DISABLE_CYCLE_COUNTER() \
    (DWT->CTRL &= (~DWT_CTRL_CYCCNTENA_Msk))

#define GET_CYCLE_COUNTER() \
    (DWT->CYCCNT)

#define RESET_CYCLE_COUNTER() \
    (DWT->CYCCNT = (0))

// TODO(Ryan): SWO and IWT

// TODO(Ryan): Using python to store code sizes in database
// https://github.com/memfault/interrupt/tree/master/example/code-size-deltas

// TODO(Ryan): NVIC registers understanding, e.g. know when there is an unhandled interrupt

INTERNAL void
__assert(u32 line, u32 *pc, u32 *lr)
{
  // TODO(Ryan): Eventually log these parameters
  BP();
}

#if defined(MAIN_DEBUG)
  // IMPORTANT(Ryan): assert() when never want to handle in production
#define ASSERT(expr)   \
  do {                         \
    if (!(expr)) {             \
      void *pc = NULL; \
      GET_PC(pc); \
      void *lr = GET_LR(); \
      __assert(__LINE__, pc, lr); \
    }                          \
  } while (0)

  #define BP() BP_IF_DEBUGGER_ATTACHED()
  #define UNREACHABLE_CODE_PATH ASSERT(!"UNREACHABLE_CODE_PATH")
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE_CODE_PATH }
#else
  #define ASSERT(expr)
  #define BP()
  #define UNREACHABLE_CODE_PATH UNREACHABLE() 
  #define UNREACHABLE_DEFAULT_CASE default: { UNREACHABLE() }
#endif
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")


#define BP_IF_DEBUGGER_ATTACHED() \
  do \
  { \
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) \
    { \
      __asm("bkpt 1"); \
    } \
  } while (0)


#define GET_LR() __builtin_return_address(0)
#define GET_PC(_a) __asm volatile ("mov %0, pc" : "=r" (_a)) 

// TODO(Ryan): What is DebugMonitor?
typedef struct __attribute__((packed)) ContextStateFrame {
  u32 r0;
  u32 r1;
  u32 r2;
  u32 r3;
  u32 r12;
  u32 lr;
  u32 return_address;
  u32 xpsr;
} sContextStateFrame;

// Disable optimizations for this function so "frame" argument
// does not get optimized away
__attribute__((optimize("O0")))
void my_fault_handler_c(sContextStateFrame *frame) {
  HALT_IF_DEBUGGING();

  // Logic for dealing with the exception. Typically:
  //  - log the fault which occurred for postmortem analysis
  //  - If the fault is recoverable,
  //    - clear errors and return back to Thread Mode
  //  - else
  //    - reboot system

  //
  // Example "recovery" mechanism for UsageFaults while not running
  // in an ISR
  // 

  // fault status register
  volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;
  const uint32_t usage_fault_mask = 0xffff0000;
  const bool non_usage_fault_occurred = (*cfsr & ~usage_fault_mask) != 0;
  // the bottom 8 bits of the xpsr hold the exception number of the
  // executing exception or 0 if the processor is in Thread mode
  const bool faulted_from_exception = ((frame->xpsr & 0xFF) != 0);
    
  if (faulted_from_exception || non_usage_fault_occurred) {
    // For any fault within an ISR or non-usage faults let's reboot the system
    volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
    *aircr = (0x05FA << 16) | 0x1 << 2;
    while (1) { } // should be unreachable
  }

  // If it's just a usage fault, let's "recover"
  // Clear any faults from the CFSR
  *cfsr |= *cfsr;
  // the instruction we will return to when we exit from the exception
  frame->return_address = (uint32_t)recover_from_task_fault;
  // the function we are returning to should never branch
  // so set lr to a pattern that would fault if it did
  frame->lr = 0xdeadbeef;
  // reset the psr state and only leave the
  // "thumb instruction interworking" bit set
  frame->xpsr = (1 << 24);
}

#define HARDFAULT_HANDLING_ASM(_x)               \
  __asm volatile(                                \
      "tst lr, #4 \n"                            \
      "ite eq \n"                                \
      "mrseq r0, msp \n"                         \
      "mrsne r0, psp \n"                         \
      "b my_fault_handler_c \n"                  \


// TODO(Ryan): put in interrupt file
INTERNAL void 
break_and_loop_fault_handler(void) 
{
  BP();
  while (1) {}
}

void trigger_irq(void) {
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (0x1 << 1);

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= (0x1 << 1);

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

void stkerr_from_psp(void) {
  extern uint32_t _start_of_ram[];
  uint8_t dummy_variable;
  const size_t distance_to_ram_bottom = (uint32_t)&dummy_variable - (uint32_t)_start_of_ram;
  volatile uint8_t big_buf[distance_to_ram_bottom - 8];
  for (size_t i = 0; i < sizeof(big_buf); i++) {
    big_buf[i] = i;
  }
  
  trigger_irq();
}

int bad_memory_access_crash(void) {
  volatile uint32_t *bad_access = (volatile uint32_t *)0xdeadbeef;
  return *bad_access;
}

int illegal_instruction_execution(void) {
  int (*bad_instruction)(void) = (void *)0xE0000000;
  return bad_instruction();
}

void unaligned_double_word_read(void) {
  extern void *g_unaligned_buffer;
  uint64_t *buf = g_unaligned_buffer;
  *buf = 0x1122334455667788;
}

void bad_addr_double_word_write(void) {
  volatile uint64_t *buf = (volatile uint64_t *)0x30000000;
  *buf = 0x1122334455667788;
}

void access_disabled_coprocessor(void) {
  // FreeRTOS will automatically enable the FPU co-processor.
  // Let's disable it for the purposes of this example
  __asm volatile(
      "ldr r0, =0xE000ED88 \n"
      "mov r1, #0 \n"
      "str r1, [r0]	\n"
      "dsb \n"
      "vmov r0, s0 \n"
      );
}

uint32_t read_from_bad_address(void) {
  return *(volatile uint32_t *)0xbadcafe;
}

void trigger_crash(int crash_id) {
  switch (crash_id) {
    case 0:
      illegal_instruction_execution();      
      break;
    case 1:
      read_from_bad_address();
      break;
    case 2:
      access_disabled_coprocessor();
      break;
    case 3:
      bad_addr_double_word_write();
      break;
    case 4:
      stkerr_from_psp();
      break;
    case 5:
      unaligned_double_word_read();      
      break;
    case 6:
      bad_memory_access_crash();
      break;
    case 7:
      trigger_irq();
      break;
    default:
      break;
  }
}


#else
#endif
