#include <iostream>

#include "many.hpp"

int main()
{
  int a(2);

  gnr::many<int, int&&, int const&&> t{1, a, 3};

  std::cout << std::get<0>(t) << std::endl;
  std::cout << std::get<1>(t) << std::endl;
  std::cout << std::get<2>(t) << std::endl;
}
