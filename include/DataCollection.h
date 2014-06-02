#ifndef DATA_COLLECTION_H
#define DATA_COLLECTION_H

#include <vector>
#include "DataPoint.h"

typedef std::vector< DataPoint2f > DataCollection;

struct DataRange
{
  DataCollection::iterator start,
    end;


  DataRange() 
  {}

  DataRange( const DataCollection::iterator& start_,
      const DataCollection::iterator& end_ ) :
    start( start_ ),
    end( end_ )
  {}

  DataRange( const DataRange& other ) :
    start( other.start ),
    end( other.end )
  {}
};

#endif
