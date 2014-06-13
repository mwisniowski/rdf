#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"

using namespace std;
class Forest 
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree& tree )
    {
      _trees.push_back( tree );
    }

    const Histogram classify( const DataPoint2f& point )
    {
      //TODO use TrainingContext
      Histogram histogram;

      vector< Tree >::iterator it = _trees.begin(),
        end = _trees.end();
      for( ; it != end; ++it )
      {
        histogram.aggregate( it->classify( point ) );
      }

      return histogram;
    }

  private:
    vector< Tree > _trees;
    size_t _numClasses;
};

#endif
