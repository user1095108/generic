#ifndef SAVE_STATE_H
# define SAVE_STATE_H
# pragma once

struct statebuf
{
  void* sp;
  void* label;
};

inline
__attribute__((always_inline))
bool getstate(statebuf& ssb) noexcept 
{
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

  asm goto (""::::rettrue);

  ssb.label = &&rettrue;

  return false;

  rettrue:
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

  return true;
}

inline void setstate(statebuf const& ssb) noexcept
{
  asm volatile (
    "movq %0, %%rsp\n\t"
    "jmp *%1"
    :
    : "m" (ssb.sp), "m" (ssb.label)
  );
}

#endif // SAVE_STATE_H
