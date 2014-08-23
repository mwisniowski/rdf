#ifndef FOREST_H
#define FOREST_H

#include "Tree.h"
#include "Interfaces.h"

template< typename D, typename F, typename S >
class Forest 
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree< D, F, S >& tree )
    {
      trees_.push_back( tree );
    }

    S classify( const TrainingContextBase< D, F, S >& context, const D& point )
    {
      S statistics = context.get_statistics();

      typename std::vector< Tree< D, F, S > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics += it->classify( point );
      }

      return statistics;
    }

  private:
    std::vector< Tree< D, F, S > >  trees_;
};

#endif
