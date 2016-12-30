#include <iostream>

#include "coroutine.hpp"

struct A
{
  ~A()
  {
    std::cout << "destroyed" << std::endl;
  }
};


int main()
{
  gnr::coroutine<> c(1024 * 1024);

  c.assign([](auto& c)
    {
      A a;

      for (int i{}; i != 3; ++i)
      {
        std::cout << i << std::endl;

        c.yield();
      }
    }
  );

  while (!c.is_terminated())
  {
    std::cout << "resuming" << std::endl;
    c.resume();
  }

  std::cin.ignore();

  return 0;
}
