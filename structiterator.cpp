#include <algorithm>

#include <execution>

#include <iostream>

#include "structiterator.hpp"

int main()
{
  struct
  {
    int a, b, c;
  } s{1, 2, 3};

  gnr::range const r(s);

  for (auto const e: r)
  {
    std::cout << e << std::endl;
  }

  int sum{};

  std::for_each(std::execution::unseq, gnr::begin(s), gnr::end(s),
    [&](auto const e) noexcept{ sum += e; });

  std::cout << sum << std::endl;

  return 0;
}
