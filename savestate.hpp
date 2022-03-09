#ifndef SAVESTATE_H
# define SAVESTATE_H
# pragma once

namespace gnr
{

struct statebuf
{
  void* sp;
  void* bp;
  void* label;
};

}

#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
  #define clobber_all() asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi");
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  #define clobber_all() asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
#elif defined(__aarch64__)
  #define clobber_all() asm volatile ("":::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x30");
#elif defined(__ARM_ARCH) && (7 == __ARM_ARCH)
  #define clobber_all() asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "lr");
#elif defined(__arm__)
  #define clobber_all() asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "lr");
#endif
#endif

#if defined(__GNUC__)
static inline bool __attribute__((always_inline)) savestate(
  gnr::statebuf& ssb) noexcept
{
  bool r;

#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "movl %%esp, %0\n\t" // store sp
    "movl %%ebp, %1\n\t" // store bp
    "movl $1f, %2\n\t" // store label
    "movb $0, %3\n\t" // return false
    "jmp 2f\n\t"
    "1:"
    "movb $1, %3\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "memory"
  );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  asm volatile (
    "movq %%rsp, %0\n\t" // store sp
    "movq %%rbp, %1\n\t" // store bp
    "lea 1f(%%rip), %%rax\n\t" // load label
    "movq %%rax, %2\n\t" // store label
    "movb $0, %3\n\t" // return false
    "jmp 2f\n\t"
    "1:"
    "movb $1, %3\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "memory", "rax"
  );
#elif defined(__aarch64__)
  asm volatile (
    "mov x7, sp\n\t"
    "str x7, %0\n\t" // store sp
    "str fp, %1\n\t" // store fp
    "ldr x7, =1f\n\t" // load label
    "str x7, %2\n\t" // store label
    "mov %w3, #0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"
    "mov %w3, #1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "x7", "memory"
  );
#elif defined(__ARM_ARCH) && (7 == __ARM_ARCH)
  asm volatile (
    "str sp, %0\n\t" // store sp
    "str r7, %1\n\t" // store fp
    "ldr r3, =1f\n\t" // load label into r3
    "str r3, %2\n\t" // store r3 into label
    "mov %3, $0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"
    "mov %3, $1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "r3", "memory"
  );
#elif defined(__arm__)
  asm volatile (
    "str sp, %0\n\t" // store sp
    "str fp, %1\n\t" // store fp
    "ldr r3, =1f\n\t" // load label into r3
    "str r3, %2\n\t" // store r3 into label
    "mov %3, $0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"
    "mov %3, $1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "r3", "memory"
  );
#endif

  return r;
}
#elif defined(_MSC_VER)
__forceinline bool savestate(gnr::statebuf& ssb) noexcept
{
  bool r;

  __asm {
    mov ebx, ssb
    mov [ebx]ssb.sp, esp
    mov [ebx]ssb.bp, ebp
    mov [ebx]ssb.label, offset _1f
    mov r, 0x0
    jmp _2f
    _1f:
    mov r, 0x1
    _2f:
  }

  return r;
}
#else
# error "unsupported compiler"
#endif

#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "movl %0, %%esp\n\t"                         \
    "movl %1, %%ebp\n\t"                         \
    "jmp *%2"                                    \
    :                                            \
    : "m" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "movq %0, %%rsp\n\t"                         \
    "movq %1, %%rbp\n\t"                         \
    "jmp *%2"                                    \
    :                                            \
    : "m" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#elif defined(__aarch64__)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "mov sp, %0\n\t"                             \
    "mov fp, %1\n\t"                             \
    "ret %2"                                     \
    :                                            \
    : "r" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#elif defined(__ARM_ARCH) && (7 == __ARM_ARCH)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "ldr sp, %0\n\t"                             \
    "mov r7, %1\n\t"                             \
    "mov pc, %2"                                 \
    :                                            \
    : "m" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#elif defined(__arm__)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "ldr sp, %0\n\t"                             \
    "mov fp, %1\n\t"                             \
    "mov pc, %2"                                 \
    :                                            \
    : "m" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#else
# error "unsupported architecture"
#endif
#elif defined(_MSC_VER)
#define restorestate(SSB)   \
  __asm mov ebx, this       \
  __asm add ebx, [SSB]      \
  __asm mov esp, [ebx]SSB.sp\
  __asm mov ebp, [ebx]SSB.bp\
  __asm jmp [ebx]SSB.label
#else
# error "unsupported compiler"
#endif

#endif // SAVESTATE_H
