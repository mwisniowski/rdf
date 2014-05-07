#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"

class Forest 
{
  public:
    Forest() {}
    virtual ~Forest() {}

  private:
    vector< Tree > _trees;
};

#endif
