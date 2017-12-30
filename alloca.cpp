#include <iostream>

#include <vector>

#include "alloca.hpp"

int main()
{
  std::vector<char> c{'H', 'e', 'l', 'l', 'o'};

  gnr::c_str(c,
    [](char const* const p) noexcept{ std::cout << p << std::endl; }
  );

  return 0;
}
