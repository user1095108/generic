#include <iostream>

#include "many.hpp"

int main()
{
  auto t(gnr::many<int, int, int>(1, 2, 3));

  std::cout << std::get<2>(t) << std::endl;

  return 0;
}
