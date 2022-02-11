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
    [](auto const i) { std::cout << i << std::endl; }
  );

  std::cout << noexcept(gnr::chain_apply(0, inc, inc, inc, inc)) << std::endl;

  return 0;
}
