#include <iostream>

#include "cify.hpp"

int main()
{
  int a = 10;

  auto f(gnr::cify([a]{std::cout << a << std::endl;}));

  f();

  return 0;
}
