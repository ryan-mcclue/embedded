// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_CONTEXT_H)
#define BASE_CONTEXT_H

// NOTE(Ryan): Compiler specifics
#if defined(__GNUC__)
  #if __GNUC__ < 10
    // no static analysis for arm-none-eabi-gcc
    #warning GCC 10+ required for builtin static analysis
  #endif

  #define COMPILER_GCC 1

  #if (defined(__arm__) || defined(__aarch64__)) && !defined(__gnu_linux__)
    #define OS_ARM_EABI 1
  #else
    #error 
  #endif

  // NOTE(Ryan): Architecture determination
  #if defined(__arm__)
    #define ARM_32 1
    #if defined(__ARM_ARCH_7A__)
    #endif
  #else
    #error Arch not supported
  #endif

  // IMPORTANT(Ryan): No sanitisers for arm-none-eabi-gcc

  #define CASE_FALLTHROUGH __attribute__((fallthrough))

  #define NEVER_INLINE   __attribute__((noinline))
  #define USED __attribute__((used,noinline))
  #define ALWAYS_INLINE __attribute__((optimize("inline-functions"),always_inline))
  
  #define UNREACHABLE() __builtin_unreachable()

  #define WEAK __attribute__((weak))

  #define LIKELY(x)   __builtin_expect(!!(x), 1) 
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)

  #define PUSH_OPTIMISATION_MODE() \
    _Pragma("GCC push_options") \
    _Pragma("GCC optimize (\"O3\")")

  #define POP_OPTIMISATION_MODE() \
    _Pragma("GCC pop_options")

  #define IGNORE_WARNING_PADDED() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wpadded\"")

  #define IGNORE_WARNING_PEDANTIC() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wpedantic\"")

  #define IGNORE_WARNING_POP() \
    _Pragma("GCC diagnostic pop")

  // TODO(Ryan): tail cail compiler macro?
  // https://blog.reverberate.org/2021/04/21/musttail-efficient-interpreters.html

  // TODO(Ryan): Perhaps also do #define MACRO_BEGIN ({
  // for compiler specific expression statements
  
  #define THREAD_LOCAL __thread
    // TODO(Ryan): Synchronisation atomics, e.g:
    // #define AtomicAdd64(ptr, v) _InterlockedExchangeAdd64((ptr), (v))
    // #define MEMORY_BARRIER()
    
    // IMPORTANT(Ryan): ARM will crash resulting from unaligned access to multibyte values from program memory
    // The const part helps compiler to place in .text section
     // #define PROGMEM const __attribute__ ((aligned(4)))

    // TODO(Ryan): SSE intrinsics
    
    // delay_cycles() function with inline assembly

  // NOTE(Ryan): C++/C determination
  #if defined(__cplusplus)
    #if __cplusplus <= 199711L
      #define CPP_VERSION 98
    #elif __cplusplus <= 201103L
      #define CPP_VERSION 11
    #endif
    #define LANG_CPP 1
  #else
    #if __STDC_VERSION__ <= 199901L
      #define C_VERSION 99
    #elif __STDC_VERSION__ <= 201112L
      #define C_VERSION 11
    #endif
    #define LANG_C 1
  #endif
  
  // NOTE(Ryan): C++/C specifics
  #if LANG_CPP
    // NOTE(Ryan): Avoid confusing auto-indenter
    // TODO: If on windows, require dll specifier
    #define EXPORT_BEGIN extern "C" {
    #define EXPORT_END }
    #define EXPORT extern "C"
    #define ZERO_STRUCT {}
    #define RESTRICT __restrict__
    #define LITERAL(t) t
  #else
    #define EXPORT_BEGIN
    #define EXPORT_END
    #define EXPORT
    #define RESTRICT restrict
    #define ZERO_STRUCT {0}
    #define LITERAL(t) (t)
  #endif

#else
  #error Compiler not supported
#endif

#endif
