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
      trees.push_back( tree );
    }

    const Histogram classify( const DataRange2f::point_type& point )
    {
      //TODO use TrainingContext
      Histogram histogram;

      vector< Tree >::iterator it = trees.begin(),
        end = trees.end();
      for( ; it != end; ++it )
      {
        histogram.aggregate( it->classify( point ) );
      }

      return histogram;
    }

  private:
    vector< Tree > trees;
    size_t numClasses;
};

#endif
