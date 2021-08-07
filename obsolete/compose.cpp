#include <iostream>

#include "compose.hpp"

int main(int, char*[])
{
  auto f0([]() noexcept
    {
      ::std::cout << "0" << ::std::endl;
    }
  );

  auto f1([]() noexcept
    {
      ::std::cout << "1" << ::std::endl;
    }
  );

  auto f2([]() noexcept
    {
      ::std::cout << "2" << ::std::endl;
    }
  );

  (f0 | f1 | f2)();

  auto a([](int v) noexcept
    {
      ::std::cout << v << ::std::endl;

      return v + 1;
    }
  );

  (a | a | a | a | a | a)(0);

  return 0;
}
