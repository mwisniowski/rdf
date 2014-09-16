#ifndef RDF_FOREST_H
#define RDF_FOREST_H

#include "Tree.h"
#include "Interfaces.h"

template< typename I, typename S, typename T >
class Forest 
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree< I, S, T >& tree )
    {
      trees_.push_back( tree );
    }

    void operator()( S& statistics, const I& input )
    {
      typename std::vector< Tree< I, S, T > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics += it->operator()( input );
      }
    }

  private:
    std::vector< Tree< I, S, T > >  trees_;
};

#endif
