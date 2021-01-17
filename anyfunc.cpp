#include <iostream>

#include <memory>

#include "anyfunc.hpp"

struct S
{
  void apply()
  {
    std::cout << "applied" << std::endl;
  }
};

int main()
{
  gnr::anyfunc<> f(
    []()
    {
      std::cout << "hello world" << std::endl;

      return 10;
    }
  );

  std::cout << f.template operator()<int>() << std::endl;

  f = [](int a, int b, int c)
    {
      std::cout << a << " " << b << " " << c << std::endl;
    };

  f.invoke<void>(1, 2, 3);

  S s;

  f = &S::apply;

  f.invoke<void>(std::ref(s));

  f = [](std::shared_ptr<int> const a)
    {
      std::cout << *a << std::endl;
    };

  f.invoke<void>(std::make_shared<int>(10));

  return 0;
}
