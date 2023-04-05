// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_TYPES_H)
#define BASE_TYPES_H

// TODO(Ryan): Investigate using gcc extensions for safer macros.
// Do they add any overhead?
#include <stdint.h>
typedef int8_t i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
// TODO(Ryan): It seems that on embedded, using say 'uint_fast8_t' can provide information to compiler to possibly
// use a register to hold the value for say array index incrementing
typedef uint8_t u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;
typedef float f32;
typedef double f64;

#include <stdbool.h>

#define GLOBAL static
#define LOCAL_PERSIST static
#if !defined(TEST_BUILD)
 #define INTERNAL static
#else
  #define INTERNAL
#endif

// TODO(Ryan): Does adding const place in flash?
// #define GLOBAL_CONST static const

#if defined(TEST_BUILD)
  GLOBAL u32 global_forever_counter = 1;
  #define FOREVER (global_forever_counter--) 
#else
  #define FOREVER 1
#endif

#if defined(SIMULATOR_BUILD) || defined(TEST_BUILD)
  #define WANT_MOCKS 1
#endif


// TODO(Ryan): Seems use global variables for constants, macros for functions? we get added type safety and can get pointer to them
// we know that any compile time constants will save RAM. compiler optimisation should save codespace also
// IMPORTANT(Ryan): C99 diverged from C++ C, so as these defined in C99, perhaps not in C++
GLOBAL i8 MIN_S8 = (i8)0x80; 
GLOBAL i16 MIN_S16 = (i16)0x8000; 
GLOBAL i32 MIN_S32 = (i32)0x80000000; 
GLOBAL i64 MIN_S64 = (i64)0x8000000000000000ll; 

GLOBAL i8 MAX_S8 = (i8)0x7f; 
GLOBAL i16 MAX_S16 = (i16)0x7fff; 
GLOBAL i32 MAX_S32 = (i32)0x7fffffff; 
GLOBAL i64 MAX_S64 = (i64)0x7fffffffffffffffll; 

GLOBAL u8 MAX_U8 = (u8)0xff; 
GLOBAL u16 MAX_U16 = (u16)0xffff; 
GLOBAL u32 MAX_U32 = (u32)0xffffffff; 
GLOBAL u64 MAX_U64 = (u64)0xffffffffffffffffllu; 

// IMPORTANT(Ryan): GCC will have the enum size accomodate the largest member
#define ENUM_U32_SIZE 0xffffffff

// NOTE(Ryan): IEEE float 7 decimal places, double 15 decimal places
GLOBAL f32 MACHINE_EPSILON_F32 = 1.1920929e-7f;
GLOBAL f32 PI_F32 = 3.1415926f;
GLOBAL f32 TAU_F32 = 6.2831853f;
GLOBAL f32 E_F32 = 2.7182818f;
GLOBAL f32 GOLD_BIG_F32 = 1.6180339f;
GLOBAL f32 GOLD_SMALL_F32 = 0.6180339f;

GLOBAL f64 MACHINE_EPSILON_F64 = 2.220446049250313e-16;
GLOBAL f64 PI_F64 =  3.141592653589793;
GLOBAL f64 TAU_F64 = 6.283185307179586;
GLOBAL f64 E_F64 =        2.718281828459045;
GLOBAL f64 GOLD_BIG_F64 = 1.618033988749894;
GLOBAL f64 GOLD_SMALL_F64 = 0.618033988749894;

GLOBAL u64 BITMASKS[65] = { 
    0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F,
    0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF,
    0xFFFF, 0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF, 0x1FFFFF, 0x3FFFFF, 0x7FFFFF,
    0xFFFFFF, 0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
    0xFFFFFFFF, 0x1FFFFFFFF, 0x3FFFFFFFF, 0x7FFFFFFFF, 0xFFFFFFFFF, 0x1FFFFFFFFF, 0x3FFFFFFFFF, 0x7FFFFFFFFF,
    0xFFFFFFFFFF, 0x1FFFFFFFFFF, 0x3FFFFFFFFFF, 0x7FFFFFFFFFF, 0xFFFFFFFFFFF, 0x1FFFFFFFFFFF, 0x3FFFFFFFFFFF, 0x7FFFFFFFFFFF,
    0xFFFFFFFFFFFF, 0x1FFFFFFFFFFFF, 0x3FFFFFFFFFFFF, 0x7FFFFFFFFFFFF, 0xFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFF, 0x3FFFFFFFFFFFFF, 0x7FFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 
    0xFFFFFFFFFFFFFFFF,
};

GLOBAL u32 BIT_1  = 1 << 0;
GLOBAL u32 BIT_2  = 1 << 1;
GLOBAL u32 BIT_3  = 1 << 2;
GLOBAL u32 BIT_4  = 1 << 3;
GLOBAL u32 BIT_5  = 1 << 4;
GLOBAL u32 BIT_6  = 1 << 5;
GLOBAL u32 BIT_7  = 1 << 6;
GLOBAL u32 BIT_8  = 1 << 7;
GLOBAL u32 BIT_9  = 1 << 8;
GLOBAL u32 BIT_10 = 1 << 9;
GLOBAL u32 BIT_11 = 1 << 10;
GLOBAL u32 BIT_12 = 1 << 11;
GLOBAL u32 BIT_13 = 1 << 12;
GLOBAL u32 BIT_14 = 1 << 13;
GLOBAL u32 BIT_15 = 1 << 14;
GLOBAL u32 BIT_16 = 1 << 15;
GLOBAL u32 BIT_17 = 1 << 16;
GLOBAL u32 BIT_18 = 1 << 17;
GLOBAL u32 BIT_19 = 1 << 18;
GLOBAL u32 BIT_20 = 1 << 19;
GLOBAL u32 BIT_21 = 1 << 20;
GLOBAL u32 BIT_22 = 1 << 21;
GLOBAL u32 BIT_23 = 1 << 22;
GLOBAL u32 BIT_24 = 1 << 23;
GLOBAL u32 BIT_25 = 1 << 24;
GLOBAL u32 BIT_26 = 1 << 25;
GLOBAL u32 BIT_27 = 1 << 26;
GLOBAL u32 BIT_28 = 1 << 27;
GLOBAL u32 BIT_29 = 1 << 28;
GLOBAL u32 BIT_30 = 1 << 29;
GLOBAL u32 BIT_31 = 1 << 30;
GLOBAL u32 BIT_32 = (u32)1 << 31;

// NOTE(Ryan): Taken from https://docs.oracle.com/cd/E19205-01/819-5265/bjbeh/index.html
INTERNAL f32
inf_f32(void)
{
  u32 temp = 0x7f800000;
  return *(f32 *)(&temp);
}

INTERNAL f32
neg_inf_f32(void)
{
  u32 temp = 0xff800000;
  return *(f32 *)(&temp);
}

INTERNAL f64
inf_f64(void)
{
  u64 temp = 0x7ff0000000000000;
  return *(f64 *)(&temp);
}

INTERNAL f64
neg_inf_f64(void)
{
  u64 temp = 0xfff0000000000000;
  return *(f64 *)(&temp);
}

INTERNAL f32  
abs_f32(f32 x)
{
  // just setting sign bit
  u32 temp = *(u32 *)(&x);
  temp &= 0x7fffffff;
  return *(f32 *)(&temp);
}

INTERNAL f64  
abs_f64(f64 x)
{
  u64 temp = *(u64 *)(&x);
  temp &= 0x7fffffffffffffff;
  return *(f64 *)(&temp);
}

typedef struct SourceLoc SourceLoc;
struct SourceLoc
{
  const char *file_name;
  const char *function_name;
  u64 line_number;
};
#define SOURCE_LOC { __FILE__, __func__, __LINE__ }
#define LITERAL_SOURCE_LOC LITERAL(SourceLoc) SOURCE_LOC 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

INTERNAL void
__assert(u32 line, u32 *pc, u32 *lr)
{
  // TODO(Ryan): Eventually log these parameters
  BP();
}

// TODO(Ryan): Using python to store code sizes in database
// https://github.com/memfault/interrupt/tree/master/example/code-size-deltas

// TODO(Ryan): NVIC registers understanding, e.g. know when there is an unhandled interrupt

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
                                                 )


// TODO(Ryan): Perhaps have DEBUG_STR("hi") "" to remove costly string literals in debug
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

#define STATIC_ASSERT(cond, line) typedef u8 PASTE(line, __LINE__) [(cond)?1:-1]
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

// to get (count, array); use to pass array inline to function
#define ARRAY_EXPAND(type, ...) ARRAY_COUNT(((type[]){ __VA_ARGS__ })), (type[]){ __VA_ARGS__ }

// NOTE(Ryan): Avoid having to worry about pernicous macro expansion
#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

// IMPORTANT(Ryan): Cannot paste token delimiters like '.', '!' etc. so cannot do 'a. ## b'
#define PASTE_(a, b) a##b
#define PASTE(a, b) PASTE_(a, b)

#define PAD(n) char PASTE(pad, __LINE__)[n]

#define UNIQUE_NAME(name) PASTE(name, __LINE__)

#define DEFER_LOOP(begin, end) \
  for (int UNIQUE_NAME(var) = (begin, 0); \
       UNIQUE_NAME(var) == 0; \
       UNIQUE_NAME(var) += 1, end)
#define DEFER_LOOP_CHECKED(begin, end) \
  for (int UNIQUE_NAME(var) = 2 * !(begin); \
       (UNIQUE_NAME(var) == 2 ? ((end), 0) : !UNIQUE_NAME(var)); \
       UNIQUE_NAME(var) += 1, (end))
#define SCOPED(end) \
  for (int UNIQUE_NAME(var) = 0; \
       UNIQUE_NAME(var) == 0; \
       UNIQUE_NAME(var) += 1, end)

// TODO: MEM_SCOPED() which encases a scratch arena

// IMPORTANT(Ryan): Maybe have to do (void)sizeof(name) for C++?
#define IGNORED(name) (void)(name) 

#define SWAP(t, a, b) do { t PASTE(temp__, __LINE__) = a; a = b; b = PASTE(temp__, __LINE__); } while(0)

#define DEG_TO_RAD(v) ((PI_F32 / 180.0f) * (v))
#define RAD_TO_DEG(v) ((180.0f / PI_F32) * (v))

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define INT_FROM_PTR(p) ((uintptr_t)((char *)p - (char *)0))
#define PTR_FROM_INT(n) ((void *)((char *)0 + (n)))

#define ABSTRACT_MEMBER(s, member) (((s *)0)->member)
#define OFFSET_OF_MEMBER(s, member) INT_FROM_PTR(&ABSTRACT_MEMBER(s, member))
#define CAST_FROM_MEMBER(S,m,p) (S*)(((u8*)p) - OFFSET_OF_MEMBER(S,m))

#define SET_FLAG(field, flag) ((field) |= (flag))
#define REMOVE_FLAG(field, flag) ((field) &= ~(flag))
#define TOGGLE_FLAG(field, flag) ((field) ^= (flag))
#define HAS_FLAGS_ANY(field, flags) (!!((field) & (flags)))
#define HAS_FLAGS_ALL(field, flags) (((field) & (flags)) == (flags))

#define SIGN_OF(x) ((x > 0) - (x < 0))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(min,x,max) (((x)<(min))?(min):((max)<(x))?(max):(x))
#define CLAMP_TOP(a,b) MIN(a,b)
#define CLAMP_BOTTOM(a,b) MAX(a,b)

#define IS_POW2_ALIGNED(x, p) (((x) & ((p) - 1)) == 0)
#define IS_POW2(x) IS_POW2_ALIGNED(x, x) 
// -x == ~(x - 1)?
#define ALIGN_POW2_DOWN(x, p)       ((x) & -(p))
#define ALIGN_POW2_UP(x, p)       (-(-(x) & -(p)))
#define ALIGN_POW2_INCREASE(x, p)         (-(~(x) & -(p)))

#define THOUSAND(x) ((x)*1000LL)
#define MILLI_TO_SEC(x) ((x)*1000ULL)

#define MILLION(x)  ((x)*1000000LL)
#define MICRO_TO_SEC(x)  ((x)*1000000ULL)

#define BILLION(x)  ((x)*1000000000LL)
#define NANO_TO_SEC(x)  ((x)*1000000000ULL)

#define TRILLION(x) ((x)*1000000000000LL)
#define PICO_TO_SEC(x) ((x)*1000000000000ULL)

#define INC_SATURATE_U8(x) ((x) = ((x) >= (MAX_U8) ? (MAX_U8) : (x + 1)))
#define INC_SATURATE_U16(x) ((x) = ((x) >= (MAX_U16) ? (MAX_U16) : (x + 1)))
#define INC_SATURATE_U32(x) ((x) = ((x) >= (MAX_U32) ? (MAX_U32) : (x + 1)))

#include <stdio.h>
#define PRINT_INT(i) printf("%s = %d\n", STRINGIFY(i), (int)(i))

// IMPORTANT(Ryan): Better than templates as no complicated type checking or generation of little functions
#define DLL_PUSH_FRONT(first, last, node) \
(\
  ((first) == NULL) ? \
  (\
    ((first) = (last) = (node)), \
    ((node)->next = (node)->prev = NULL) \
  )\
  : \
  (\
    ((node)->prev = NULL), \
    ((node)->next = (first)), \
    ((first)->prev = (node)), \
    ((first) = (node)) \
  )\
)
  

#define DLL_PUSH_BACK(first, last, node) \
(\
  ((first) == NULL) ? \
  (\
    ((first) = (last) = (node)), \
    ((node)->next = (node)->prev = NULL) \
  )\
  : \
  (\
    ((node)->prev = (last)), \
    ((node)->next = NULL), \
    ((last)->next = (node)), \
    ((last) = (node)) \
  )\
)

#define DLL_REMOVE(first, last, node) \
(\
  ((node) == (first)) ? \
  (\
    ((first) == (last)) ? \
    (\
      ((first) = (last) = NULL) \
    )\
    : \
    (\
      ((first) = (first)->next), \
      ((first)->prev = NULL) \
    )\
  )\
  : \
  (\
    ((node) == (last)) ? \
    (\
      ((last) = (last)->prev), \
      ((last)->next = NULL) \
    )\
    : \
    (\
      ((node)->next->prev = (node)->prev), \
      ((node)->prev->next = (node)->next) \
    )\
  )\
)

#define SLL_QUEUE_PUSH(first, last, node) \
(\
  ((first) == NULL) ? \
   (\
    ((first) = (last) = (node)), \
    ((node)->next = NULL) \
   )\
  : \
  (\
    ((last)->next = (node)), \
    ((last) = (node)), \
    ((node)->next = NULL) \
  )\
)

#define SLL_QUEUE_POP(first, last) \
(\
  ((first) == (last)) ? \
    (\
     ((first) = (last) = NULL) \
    ) \
  : \
  (\
    ((first) = (first)->next) \
  )\
)

#define SLL_STACK_PUSH(first, node) \
(\
  ((node)->next = (first)), \
  ((first) = (node)) \
)

#define SLL_STACK_POP(first) \
(\
  ((first) != NULL) ? \
    (\
     ((first) = (first)->next) \
    )\
)

#endif


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
