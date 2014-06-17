#ifndef DATA_RANGE_H
#define DATA_RANGE_H

#include <vector>
#include "DataPoint.h"

/**
 * @brief Stores start and end iterator of typename vector< DataPoint< I, O, s > >
 */
template< typename D >
struct DataRange
{
  typedef vector< D >                           collection;
  typedef typename collection::iterator         iterator;
  typedef typename collection::const_iterator   const_iterator;

  iterator start,
           end;

  DataRange() 
  {}

  DataRange( const iterator& start_, const iterator& end_ ) :
    start( start_ ),
    end( end_ )
  {}

  DataRange( const DataRange& other ) :
    start( other.start ),
    end( other.end )
  {}
};

#endif
