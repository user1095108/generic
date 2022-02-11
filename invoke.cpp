#include <iostream>

#include "invoke.hpp"

int main()
{
  auto const inc([](auto const i) noexcept
    {
      return i + 1;
    }
  );

  std::cout << gnr::chain_apply(0, inc, inc, inc, inc) << std::endl;
  std::cout << noexcept(gnr::chain_apply(0, inc, inc, inc, inc)) << std::endl;

  return 0;
}
