#ifndef DATA_RANGE_H
#define DATA_RANGE_H

#include <vector>
#include "DataPoint.h"

/**
 * @brief Stores start and end iterator of typename vector< DataPoint< I, O, s > >
 */
template< typename D >
class DataRange
{
  public:
    typedef vector< D >                           collection;
    typedef typename collection::iterator         iterator;
    typedef typename collection::const_iterator   const_iterator;


  private:
    iterator from, to;


  public:
    DataRange() 
    {}

    DataRange( const iterator& begin, const iterator& end ) :
      from( begin ),
      to( end )
    {}

    DataRange( const DataRange& other ) :
      from( other.from ),
      to( other.to )
    {}

    iterator begin() const
    {
      return from;
    }

    iterator end() const
    {
      return to;
    }
};

#endif
