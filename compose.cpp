#include <iostream>

#include "compose.hpp"

int main(int, char*[])
{
  auto a([](int v) noexcept
    {
      ::std::cout << v << ::std::endl;

      return v + 1;
    }
  );

  (a | a | a | a)(0);

  return 0;
}
