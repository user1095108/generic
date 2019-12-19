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
  static_assert(noexcept(f(s)));

  f(s);

  gnr::forwarder<void(S const*) noexcept> g(&S::f);
  static_assert(noexcept(g(&s)));

  g(&s);

  gnr::forwarder<void()> h([&]() { s.f(); });
  static_assert(!noexcept(h()));

  h();

  return 0;
}
