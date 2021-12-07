#include <iostream>

#include <vector>

#include "alloca.hpp"

int main()
{
  auto const l(
    [](char const* const p) noexcept{ std::cout << p << std::endl; }
  );

  std::vector<char> c{'H', 'e', 'l', 'l', 'o'};

  gnr::c_str(c, l);

  std::string_view s("Hello");

  gnr::c_str(s, l);

  return 0;
}
