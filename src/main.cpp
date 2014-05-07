#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "Instance.h"
#include "RandomTree.h"

using namespace std;
typedef Instance< float, int, 2, 1 > Instance2f;

int main(int argc, char *argv[])
{
  if( argc <= 1 )
  {
    cerr << "Path to CSV required!" << endl;
    return 1;
  }
  ifstream is( argv[1] );

  istream_iterator< Instance2f > start( is ), end;

  vector< Instance2f > v( start, end );
  RandomTree rt( v );

  return 0;
}
