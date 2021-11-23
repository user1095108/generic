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

  std::cout <<
    gnr::dispatch2(
      state = INVALID,
      NORMAL,
      []() -> decltype(auto) { return "NORMAL"; },
      INVALID,
      []() -> decltype(auto) { return "INVALID"; }
    ) <<
    std::endl;

  return 0;
}
