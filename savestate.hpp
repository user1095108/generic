#ifndef CR_SAVESTATE_HPP
# define CR_SAVESTATE_HPP
# pragma once

namespace gnr
{

struct statebuf
{
  void* sp, *label;
#if defined(i386) || defined(__i386) || defined(__i386__)
  void* bp;
#endif
};

}

#if defined(__GNUC__)
# if defined(i386) || defined(__i386) || defined(__i386__)
#  define clobber_all() asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "cc");
# elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define clobber_all() asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "cc");
# elif defined(__arm__)
#  define clobber_all() asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr", "cc");
# elif defined(__aarch64__)
#  define clobber_all() asm volatile ("":::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "cc");
# else
#  error "unsupported architecture"
# endif
#else
# error "unsupported compiler"
#endif

#if defined(__GNUC__)
static inline bool __attribute__((always_inline)) savestate(
  gnr::statebuf& ssb) noexcept
{
  bool r;

# if defined(i386) || defined(__i386) || defined(__i386__)
  asm (
    "movl %%esp, %0\n\t" // store sp
    "movl %%ebp, %1\n\t" // store bp
    "lea 1f, %%eax\n\t" // load label
    "movl %%eax, %2\n\t" // store label
    "movb $0, %3\n\t" // return false
    "jmp 2f\n\t"
    "1:"
    "movb $1, %3\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "eax", "memory"
  );
# elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
   defined(__x86_64)
  asm (
    "movq %%rsp, %0\n\t" // store sp
    "lea 1f(%%rip), %%rax\n\t" // load label
    "movq %%rax, %1\n\t" // store label
    "movb $0, %2\n\t" // return false
    "jmp 2f\n\t"
    "1:"
    "movb $1, %2\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.label), "=r" (r)
    :
    : "rax", "memory"
  );
# elif defined(__arm__)
  asm (
    "str sp, %0\n\t" // store sp
    "ldr r0, =1f\n\t" // load label
    "str r0, %1\n\t" // store label
    "mov %2, $0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"
    "mov %2, $1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.label), "=r" (r)
    :
    : "r0", "memory"
  );
# elif defined(__aarch64__)
  asm (
    "mov x0, sp\n\t"
    "str x0, %0\n\t" // store sp
    "ldr x0, =1f\n\t" // load label
    "str x0, %1\n\t" // store label
    "mov %w2, #0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"
    "mov %w2, #1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.label), "=r" (r)
    :
    : "x0", "memory"
  );
# else
#  error "unsupported architecture"
# endif

  return r;
}
#else
# error "unsupported compiler"
#endif

#if defined(__GNUC__)
# if defined(i386) || defined(__i386) || defined(__i386__)
#  define restorestate(SSB)                        \
    asm (                                          \
      "movl %0, %%esp\n\t"                         \
      "movl %1, %%ebp\n\t"                         \
      "jmp *%2"                                    \
      :                                            \
      : "m" (SSB.sp), "m" (SSB.bp), "m" (SSB.label)\
    );                                             \
    __builtin_unreachable();
# elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
   defined(__x86_64)
#  define restorestate(SSB)          \
    asm (                            \
      "movq %0, %%rsp\n\t"           \
      "jmp *%1"                      \
      :                              \
      : "m" (SSB.sp), "m" (SSB.label)\
    );                               \
    __builtin_unreachable();
# elif defined(__arm__)
#  define restorestate(SSB)          \
    asm (                            \
      "mov sp, %0\n\t"               \
      "mov pc, %1"                   \
      :                              \
      : "r" (SSB.sp), "r" (SSB.label)\
    );                               \
    __builtin_unreachable();
# elif defined(__aarch64__)
#  define restorestate(SSB)          \
    asm (                            \
      "mov sp, %0\n\t"               \
      "ret %1"                       \
      :                              \
      : "r" (SSB.sp), "r" (SSB.label)\
    );                               \
    __builtin_unreachable();
# else
#  error "unsupported architecture"
# endif
#else
# error "unsupported compiler"
#endif

#endif // CR_SAVESTATE_HPP
