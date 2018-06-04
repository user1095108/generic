#include <iostream>

#include <vector>

#include "rev.hpp"

int main(int, char*[])
{
  for (auto i: gnr::rev(std::vector<int>{1, 2, 3, 4}))
  {
    std::cout << i << std::endl;
  }
}
