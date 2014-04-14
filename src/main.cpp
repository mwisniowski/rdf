#include <iostream>
#include <fstream>
#include "Instance2f.h"

int main(int argc, char *argv[])
{
  std::ifstream is("test.csv");
  std::istream_iterator< Instance2f > start( is ), end;

  for (; start != end; start++)
  {
    std::cout << *start << std::endl;
  }

  return 0;
}
