#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"
#include "Interfaces.h"

using namespace std;

template< typename D, typename F, typename S >
class Forest 
{
  private:
    vector< Tree< D, F, S > >           trees;
    const ITrainingContext< D, F, S >&  context;

  public:
    Forest( const ITrainingContext< D, F, S >& context ) :
      context( context )
    {}

    virtual ~Forest() {}

    void add( const Tree< D, F, S >& tree )
    {
      trees.push_back( tree );
    }

    const S classify( const D& point )
    {
      S statistics = context.get_statistics();

      typename vector< Tree< D, F, S > >::iterator it = trees.begin(),
        end = trees.end();
      for( ; it != end; ++it )
      {
        statistics += it->classify( point );
      }

      return statistics;
    }

};

#endif
