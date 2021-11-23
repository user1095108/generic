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

  std::cout << gnr::select(state, "NORMAL", "INVALID") << std::endl;

  std::cout <<
    gnr::dispatch2(
      state = INVALID,
      NORMAL,
      []{ return "NORMAL"; },
      INVALID,
      []{ return "INVALID"; }
    ) <<
    std::endl;

  std::cout <<
    gnr::select2(
      state,
      NORMAL,
      "NORMAL",
      INVALID,
      "INVALID"
    ) <<
    std::endl;

  return 0;
}
