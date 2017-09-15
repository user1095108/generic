#include <iostream>

#include <vector>

#include "anyfind.hpp"

int main()
{
  int a[] = {1, 2, 3, 4, 5};

  std::cout << *gnr::any_find(a, 3).value() << std::endl;

  int b[][2] = {
    {1, 2},
    {3, 4},
    {5, 6}
  };

  std::cout << *gnr::any_find(b, 2).value() << std::endl;

  std::vector<std::string> v{
    "a",
    "b",
    "c"
  };

  std::cout << *gnr::any_find(v, "c").value() << std::endl;
}
