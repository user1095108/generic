#include <iostream>

#include "cify.hpp"

int main()
{
  int a = 10;

  auto f(gnr::cify([a]{std::cout << a << std::endl;}));

  f();

  f = gnr::cify_once([&a]{std::cout << ++a << std::endl;});

  f();

  return 0;
}
