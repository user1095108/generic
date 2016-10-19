#ifndef SAVE_STATE_H
# define SAVE_STATE_H
# pragma once

struct statebuf
{
  void* sp;
  void* label;
};

inline bool savestate(statebuf& ssb) noexcept 
{
  bool r;

#if defined(i386) || defined(__i386) || defined(__i386__)
  asm volatile (
    "movl %%esp, %0\n\t" // store sp
    "movl $1f, %1\n\t" // store label
    "movb $0, %2\n\t" // return false
    "jmp 2f\n\t"
    "1:movb $1, %2\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.label), "=r" (r)
    :
    : "memory"
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  asm volatile (
    "movq %%rsp, %0\n\t" // store sp
    "movq $1f, %1\n\t" // store label
    "movb $0, %2\n\t" // return false
    "jmp 2f\n\r"
    "1:movb $1, %2\n\t" // return true
    "2:"
    : "=m" (ssb.sp), "=m" (ssb.label), "=r" (r)
    :
    : "memory"
#else
# error ""
#endif
  );

  return r;
}

inline void restorestate(statebuf const& ssb) noexcept
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
