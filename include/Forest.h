#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"
#include "ITrainingContext.h"

using namespace std;

template< typename D, typename F, typename S >
class Forest 
{
  private:
    vector< Tree< D, F, S > >         trees;
    size_t                            numClasses;
    const ITrainingContext< F, S >&   context;

  public:
    Forest( const ITrainingContext< F, S >& c ) :
      context( c )
    {}

    virtual ~Forest() {}

    void add( const Tree< D, F, S >& tree )
    {
      trees.push_back( tree );
    }

    const S classify( const D& point )
    {
      S s = context.getStatisticsAggregator();

      typename vector< Tree< D, F, S > >::iterator it = trees.begin(),
        end = trees.end();
      for( ; it != end; ++it )
      {
        s.aggregate( it->classify( point ) );
      }

      return s;
    }

};

#endif
