#include <iostream>

#include "invoke.hpp"

int main()
{
  auto const inc([](auto const i) noexcept
    {
      return i + 1;
    }
  );

  gnr::chain_apply(
    0,
    inc,
    inc,
    inc,
    inc,
    [](auto const i) { std::cout << i << std::endl; },
    []{ std::cout << "lol" << std::endl; }
  );

  std::cout << noexcept(gnr::chain_apply(0, inc, inc, inc, inc)) << std::endl;

  //
  auto const fib([](auto const a, decltype(a) b) noexcept
    {
      return std::tuple(b, a + b);
    }
  );

  gnr::chain_apply(
    std::tuple(0, 1),
    fib,
    fib,
    fib,
    fib,
    [](auto, auto const b) { std::cout << b << std::endl; }
  );

  return 0;
}
