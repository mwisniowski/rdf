#include <iostream>
#include <fstream>
#include "Instance.h"

int main(int argc, char *argv[])
{
  std::ifstream is("test.csv");
  std::istream_iterator< Instance< float, int, 2, 1 > > start( is ), end;

  for (; start != end; start++)
  {
    std::cout << *start << std::endl;
  }

  return 0;
}
