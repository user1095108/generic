#include <iostream>

#include "memfun.hpp"

struct S
{
  void f() const noexcept
  {
    std::cout << "Hello world!" << std::endl;
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

  return 0;
}
