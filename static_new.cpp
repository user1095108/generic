#include <iostream>

#include "static_new.hpp"

int main()
{
  std::cout << (*gnr::static_new<int>() = 10) << std::endl;

  gnr::static_new<int[2]>();

  return 0;
}
