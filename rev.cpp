#include <iostream>

#include <vector>

#include "rev.hpp"

int main(int, char*[])
{
  for (auto& i: gnr::rev({1, 2, 3, 4}))
  {
    std::cout << i << std::endl;
    i = 0;
  }

  int const a[]{1, 2, 3, 4};

  for (auto i: gnr::rev(a))
  {
    std::cout << i << std::endl;
  }

  std::vector<int> r{1, 2, 3, 4};

  for (auto& i: gnr::rev(r))
  {
    std::cout << i << std::endl;
    i = 0;
  }

  for (auto& i: gnr::rev(std::vector<int>{1, 2, 3, 4}))
  {
    std::cout << i << std::endl;
    i = 0;
  }
}
