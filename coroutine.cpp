#include <iostream>

#include "coroutine.hpp"

struct A
{
  ~A()
  {
    ::std::cout << "destroyed" << ::std::endl;
  }
};


int main()
{
  ::generic::coroutine c(1024 * 1024);

  c.run([](decltype(c)& c)
    {
      A a;

      for (int i{}; i != 3; ++i)
      {
        ::std::cout << i << ::std::endl;

        c.yield();
      }
    }
  );

  while (c.running())
  {
    c.resume();
  }

  return 0;
}
