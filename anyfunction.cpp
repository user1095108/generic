#include <iostream>

#include "anyfunction.hpp"

int main()
{
  ::generic::any_function<> f(
    []()
    {
      ::std::cout << "hello world" << ::std::endl;

      return 10;
    }
  );

  f();

  ::std::cout << f.operator()<int>() << ::std::endl;

  return 0;
}
