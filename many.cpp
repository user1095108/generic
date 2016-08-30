#include <iostream>

#include "many.hpp"

int main()
{
  auto t(::generic::make_many(1, 2, 3));

  ::std::cout << ::std::get<2>(t) << ::std::endl;

  return 0;
}
