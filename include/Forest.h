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

    const StatisticsAggregator classify( const DataPoint2f& point )
    {
      //TODO use TrainingContext
      StatisticsAggregator statistics( _numClasses );

      vector< Tree >::iterator it = _trees.begin(),
        end = _trees.end();
      for( ; it != end; ++it )
      {
        statistics.aggregate( it->classify( point ) );
      }

      return statistics;
    }

  private:
    vector< Tree > _trees;
    size_t _numClasses;
};

#endif
