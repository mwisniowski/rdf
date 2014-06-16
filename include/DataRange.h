#ifndef DATA_RANGE_H
#define DATA_RANGE_H

#include <vector>
#include "DataPoint.h"

/**
 * @brief Stores start and end iterator of typename vector< DataPoint< I, O, s > >
 */
template< typename I, typename O, size_t d >
struct DataRange
{
  typedef DataPoint< I, O, d > point_type;
  typedef vector< point_type > collection;
  typedef typename collection::iterator iterator;
  typedef typename collection::const_iterator const_iterator;

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

typedef DataRange< float, u_int, 2 > DataRange2f;

#endif
