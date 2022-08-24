#include "dbg.hpp"

int main()
{
  gnr::dbg << "test" << std::endl;
  std::cerr << "test" << gnr::abort;

  return 0;
}
