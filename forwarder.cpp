#include <cassert>

#include <iostream>

#include "forwarder.hpp"

struct S
{
  void f() const noexcept
  {
    std::cout << "Hello world!" << std::endl;
  }
};

int main()
{
  S const s;

  gnr::forwarder<void(S const&) noexcept> f(&S::f);
  assert(noexcept(f(s)));

  f(s);

  gnr::forwarder<void(S const*) noexcept> g(&S::f);

  g(&s);

  gnr::forwarder<void()> h([&]() { s.f(); });

  h();

  return 0;
}
