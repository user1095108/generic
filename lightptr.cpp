#include <iostream>

#include "lightptr.hpp"

template <typename U>
//using ptr_t = std::shared_ptr<U>;
using ptr_t = gnr::light_ptr<U>;

int main()
{
  ptr_t<int> i(new int(10));

  *i = 9;
  std::cout << *i << std::endl;

  return 0;
}
