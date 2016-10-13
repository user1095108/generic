#include <iostream>

#include "coroutine.hpp"

int main()
{
  ::generic::coroutine<> c;

  c.run([](decltype(c)& c)
    {
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
