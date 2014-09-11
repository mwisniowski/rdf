#ifndef RDF_FOREST_H
#define RDF_FOREST_H

#include "Tree.h"
#include "Interfaces.h"

template< typename F, typename I, typename S >
class Forest 
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree< F, I, S >& tree )
    {
      trees_.push_back( tree );
    }

    S classify( S& statistics, const std::vector< I >& input )
    {
      typename std::vector< Tree< F, I, S > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics += it->evaluate( input );
      }

      return statistics;
    }

  private:
    std::vector< Tree< F, I, S > >  trees_;
};

#endif
