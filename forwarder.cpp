#include <iostream>

#include "forwarder.hpp"
#include "fwdref.hpp"

struct S
{
  void f() const noexcept
  {
    std::cout << "Hello world!" << std::endl;
  }
};

void test(gnr::fwdref<void()> const f)
{
  f();
}

int main()
{
  S const s;

  gnr::forwarder<void(S const&) noexcept> f(&S::f);
  static_assert(noexcept(f(s)));

  f(s);

  gnr::forwarder<void(S const*) noexcept> g(&S::f);
  static_assert(noexcept(g(&s)));

  g(&s);

  gnr::forwarder<void()> h([&] { s.f(); });
  static_assert(!noexcept(h()));

  h();

  auto const message("");
  std::cout << std::addressof(message) << std::endl;

  test([&]{
      std::cout << std::addressof(message) << std::endl;
    }
  );

  return 0;
}
