#include <algorithm>

#include <execution>

#include <iostream>

#include "structiterator.hpp"

int main()
{
  {
    struct S
    {
      bool; int; char;
    };

    std::cout << gnr::detail::struct_iterator::is_proper_v<S> << std::endl;
  }

  {
    struct S { };

    std::cout << gnr::detail::struct_iterator::is_proper_v<S> << std::endl;
  }


  struct
  {
    int a, b, c;
  } s{1, 2, 3};

  for (auto const e: gnr::range(s))
  {
    std::cout << e << std::endl;
  }

  int sum{};

  std::for_each(std::execution::unseq, gnr::begin(s), gnr::end(s),
    [&](auto const e) noexcept{ sum += e; });

  std::cout << sum << std::endl;

  return 0;
}
