#include <iostream>

#include "anyfunction.hpp"

struct S
{
  void apply()
  {
    ::std::cout << "applied" << ::std::endl;
  }
};

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

  ::std::cout << f.invoke<int>() << ::std::endl;

  S s;

  f = &S::apply;

  f(::std::ref(s));

  return 0;
}
