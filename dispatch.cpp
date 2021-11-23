#include <iostream>

#include "dispatch.hpp"

int main()
{
  enum
  {
    NORMAL,
    INVALID
  } state(NORMAL);

  std::cout <<
    gnr::dispatch(
      state,
      []{ return "NORMAL"; },
      []{ return "INVALID"; }
    ) <<
    std::endl;

  std::cout << gnr::select(1, "NORMAL", "INVALID") << std::endl;

  return 0;
}
