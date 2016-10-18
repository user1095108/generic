#ifndef SAVE_STATE_H
# define SAVE_STATE_H
# pragma once

struct statebuf
{
  void* sp;
  void* label;
};

inline
__attribute__((always_inline, returns_twice))
bool getstate(statebuf& ssb) noexcept 
{
  ssb.label = &&rettrue;

#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "push %%eax\n\t"
    "push %%ebx\n\t"
    "push %%ecx\n\t"
    "push %%edx\n\t"
    "push %%esi\n\t"
    "push %%edi\n\t"
    "push %%ebp\n\t"
    "movl %%esp, %0\n\t"
    : "=m" (ssb.sp)
    :
    : "memory"
  );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  asm volatile (
    "push %%rax\n\t"
    "push %%rbx\n\t"
    "push %%rcx\n\t"
    "push %%rdx\n\t"
    "push %%rsi\n\t"
    "push %%rdi\n\t"
    "push %%rbp\n\t"
    "push %%r8\n\t"
    "push %%r9\n\t"
    "push %%r10\n\t"
    "push %%r11\n\t"
    "push %%r12\n\t"
    "push %%r13\n\t"
    "push %%r14\n\t"
    "push %%r15\n\t"
    "movq %%rsp, %0\n\t"
    : "=m" (ssb.sp)
    :
    : "memory"
  );
#else
# error ""
#endif

  asm goto (""::::rettrue);

  return false;

  rettrue:
#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "pop %%ebp\n\t"
    "pop %%edi\n\t"
    "pop %%esi\n\t"
    "pop %%edx\n\t"
    "pop %%ecx\n\t"
    "pop %%ebx\n\t"
    "pop %%eax\n\t"
    :
    :
  );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  asm volatile (
    "pop %%r15\n\t"
    "pop %%r14\n\t"
    "pop %%r13\n\t"
    "pop %%r12\n\t"
    "pop %%r11\n\t"
    "pop %%r10\n\t"
    "pop %%r9\n\t"
    "pop %%r8\n\t"
    "pop %%rbp\n\t"
    "pop %%rdi\n\t"
    "pop %%rsi\n\t"
    "pop %%rdx\n\t"
    "pop %%rcx\n\t"
    "pop %%rbx\n\t"
    "pop %%rax\n\t"
    :
    :
  );
#else
# error ""
#endif

  return true;
}

inline void setstate(statebuf const& ssb) noexcept
{
#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "movl %0, %%esp\n\t"
    "jmp *%1"
    :
    : "m" (ssb.sp), "m" (ssb.label)
  );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  asm volatile (
    "movq %0, %%rsp\n\t"
    "jmp *%1"
    :
    : "m" (ssb.sp), "m" (ssb.label)
  );
#else
# error ""
#endif
}

#endif // SAVE_STATE_H
