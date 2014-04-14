#include <vector>
#include <set>
#include "Instance2f.h"

class RandomTree
{
public:
  RandomTree() {}

  RandomTree( std::vector< Instance2f > data ) :
    _data( data )
  {}

  virtual ~RandomTree() {}

private:
  std::vector< Instance2f > _data;
};
