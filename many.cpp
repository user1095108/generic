#include "many.hpp"

int main()
{
  ::generic::many<int, int, int> t;

  return ::std::get<0>(t);
}
