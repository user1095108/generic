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

  gnr::struct_range const r(s);

  for (auto const& e: r)
  {
    std::cout << e << std::endl;
  }

  int sum{};

  std::for_each(std::execution::unseq, r.begin(), r.end(),
    [&](auto const e) noexcept{ sum += e; });

  std::cout << sum << std::endl;

  return 0;
}
