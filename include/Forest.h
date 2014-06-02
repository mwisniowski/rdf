#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"

using namespace std;
class Forest 
{
  public:
    Forest( size_t numClasses ) :
      _numClasses( numClasses )
    {}

    virtual ~Forest() {}

    void add( const Tree& tree )
    {
      _trees.push_back( tree );
    }

    const Histogram classify( const DataPoint2f& point )
    {
      //TODO use TrainingContext
      Histogram histogram( _numClasses );

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
