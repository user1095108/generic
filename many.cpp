#include <iostream>

#include "many.hpp"

int main()
{
  gnr::many<int, int, int> t{1, 2, 3};

  std::cout << std::get<2>(t) << std::endl;
}
