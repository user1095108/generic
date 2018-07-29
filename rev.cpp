#include <iostream>

#include <vector>

#include "rev.hpp"

int main(int, char*[])
{
  for (auto& i: gnr::rev({1, 2, 3, 4}))
  {
    std::cout << i << std::endl;
  }

  auto r(std::vector<int>{1, 2, 3, 4});

  for (auto& i: gnr::rev(r))
  {
    std::cout << i << std::endl;
    i = 0;
  }
}
