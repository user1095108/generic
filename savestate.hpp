#ifndef SAVE_STATE_H
# define SAVE_STATE_H
# pragma once

struct statebuf
{
  void* sp;
  void* bp;
  void* label;
};

#if defined(__GNUC__)
static inline bool __attribute__((always_inline)) savestate(
  statebuf& ssb) noexcept
{
  bool r;

#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "movl %%esp, %0\n\t" // store sp
    "movl %%ebp, %1\n\t" // store bp
    "movl $1f, %2\n\t" // store label
    "movb $0, %3\n\t" // return false
    "jmp 2f\n\t"
    "1:\n\t"
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
    "movq $1f, %2\n\t" // store label
    "movb $0, %3\n\t" // return false
    "jmp 2f\n\t"
    "1:\n\t"
    "movb $1, %3\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "memory"
  );
#elif defined(__arm__) && !defined(__ARM_ARCH_7__)
  asm volatile (
    "str sp, %0\n\t" // store sp
    "str fp, %1\n\t" // store fp
    "ldr r3, =1f\n\t" // load label into r3
    "str r3, %2\n\t" // store r3 into label
    "mov %3, $0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"             // restore fp
    "mov %3, $1\n\t" // store 1 into result
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.bp), "=m" (ssb.label), "=r" (r)
    :
    : "r3", "memory"
  );
#elif defined(__arm__) && defined(__ARM_ARCH_7__)
  asm volatile (
    "str sp, %0\n\t" // store sp
    "str r7, %1\n\t" // store fp
    "ldr r3, =1f\n\t" // load label into r3
    "str r3, %2\n\t" // store r3 into label
    "mov %3, $0\n\t" // store 0 into result
    "b 2f\n\t"
    "1:"             // restore fp
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
__forceinline bool savestate(statebuf& ssb) noexcept
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
#elif defined(__arm__) && !defined(__ARM_ARCH_7__)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "ldr sp, %0\n\t"                             \
    "mov fp, %1\n\t"                             \
    "mov pc, %2"                                 \
    :                                            \
    : "m" (SSB.sp), "r" (SSB.bp), "r" (SSB.label)\
  );
#elif defined(__arm__) && defined(__ARM_ARCH_7__)
#define restorestate(SSB)                        \
  asm volatile (                                 \
    "ldr sp, %0\n\t"                             \
    "mov r7, %1\n\t"                             \
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

#endif // SAVE_STATE_H
