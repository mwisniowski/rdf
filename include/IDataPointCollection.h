#ifndef IDATAPOINT_COLLECTION_H
#define IDATAPOINT_COLLECTION_H

#include <vector>
#include "DataPoint.h"

typedef std::vector< DataPoint2f > IDataPointCollection;

struct IDataPointRange
{
  IDataPointCollection::iterator start,
    end;


  IDataPointRange() 
  {}

  IDataPointRange( const IDataPointCollection::iterator& start_,
      const IDataPointCollection::iterator& end_ ) :
    start( start_ ),
    end( end_ )
  {}

  IDataPointRange( const IDataPointRange& other ) :
    start( other.start ),
    end( other.end )
  {}
};

#endif
