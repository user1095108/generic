#include <iostream>

#include "memfun.hpp"

struct S
{
  void f() const noexcept
  {
    std::cout << "Hello world!" << std::endl;
  }

  void g(int const a) const noexcept
  {
    std::cout << a << std::endl;
  }
};


int main()
{
  S s;

  auto const f(gnr::memfun<MEMFUN(S::f)>(s));

  f();

  auto const g(gnr::memfun<MEMFUN(S::f)>(&s));

  g();

  auto const h(gnr::memfun_ref<MEMFUN(S::f)>());

  h(s);

  auto const i(gnr::memfun_ref<MEMFUN(S::g)>());

  i(s, 10);

  return 0;
}
