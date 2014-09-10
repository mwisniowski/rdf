#ifndef RDF_FOREST_H
#define RDF_FOREST_H

#include "Tree.h"
#include "Interfaces.h"

template< typename I, typename O, typename F, typename S >
class Forest 
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree< I, O, F, S >& tree )
    {
      trees_.push_back( tree );
    }

    S classify( S& statistics, const DataPoint< I, O >& point )
    {
      typename std::vector< Tree< I, O, F, S > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics += it->evaluate( point );
      }

      return statistics;
    }

  private:
    std::vector< Tree< I, O, F, S > >  trees_;
};

#endif
