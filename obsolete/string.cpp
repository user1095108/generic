#include <iostream>

#include "string.hpp"

int main(int, char* [])
{
  std::cout <<
    gnr::stoi<unsigned>(std::string_view("123")).value() <<
    std::endl;

  std::cout <<
    int(gnr::stoi<std::int8_t>("127").value()) <<
    std::endl;
}
