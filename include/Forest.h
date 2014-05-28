#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"

using namespace std;
class Forest 
{
  public:
    Forest() {}
    virtual ~Forest() {}

    void add( const Tree& tree )
    {
      _trees.push_back( tree );
    }

    pair< u_int, float > classify( const DataPoint2f& point )
    {
      StatisticsAggregator statistics;

      vector< Tree >::iterator it = _trees.begin(),
        end = _trees.end();
      for( ; it != end; ++it )
      {
        statistics.aggregate( it->classify( point ) );
      }

      u_int c = statistics.maxClass();
      return pair< u_int, float >( c, statistics.probability( c ) );
    }

  private:
    vector< Tree > _trees;
};

#endif
