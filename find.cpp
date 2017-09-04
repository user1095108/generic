#include <iostream>

#include "find.hpp"

//////////////////////////////////////////////////////////////////////////////
int main()
{
  int a[] = {1, 2, 3, 4, 5};

  std::cout << *gnr::any_find(a, 3) << std::endl;

  int b[][2] = {
    {1, 2},
    {3, 4}
  };

  std::cout << *gnr::any_find(b, 2) << std::endl;
}
