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
      []() -> decltype(auto) { return "NORMAL"; },
      []() -> decltype(auto) { return "INVALID"; }
    ) <<
    std::endl;

  std::cout << gnr::select(1, 0, 1, 4) << std::endl;

  return 0;
}
