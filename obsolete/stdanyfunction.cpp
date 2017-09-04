#include <iostream>

#include "stdanyfunction.hpp"

struct S
{
  void apply()
  {
    ::std::cout << "applied" << ::std::endl;
  }
};

int main()
{
  any_function<> f(
    []()
    {
      ::std::cout << "hello world" << ::std::endl;

      return 10;
    }
  );

  ::std::cout << ::std::experimental::any_cast<int const&>(f()) << ::std::endl;

  S s;

  f = &S::apply;

  f(::std::ref(s));

  return 0;
}
