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

    void evaluate( std::vector< const S* >& statistics, const I& input )
    {
      typename std::vector< Tree< I, S, T > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics.push_back( &( it->evaluate( input ) ) );
      }
    }

    void evaluate( S& s, const I& input )
    {
      typename std::vector< Tree< I, S, T > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        s += it->evaluate( input );
      }
    }

    std::vector< Tree< I, S, T > >  trees_;
  private:
};

#endif
