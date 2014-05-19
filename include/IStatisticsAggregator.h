#ifndef CVT_RDF_ISTATISTICS_AGGREGATOR
#define CVT_RDF_ISTATISTICS_AGGREGATOR

#include <vector>

#include "IDataPointCollection.h"
#include "DataPoint.h"

using namespace std;

class IStatisticsAggregator 
{
  public:
    virtual size_t numClasses() const =0;
    virtual float probability( size_t class_label ) const =0;
    virtual void aggregate( const DataPoint2f& point ) =0;
    virtual void aggregate( const IDataPointCollection& data )
    {
      IDataPointCollection::const_iterator it = data.begin(),
        end = data.end();
      for( ; it != end; ++it )
      {
        aggregate( *it );
      }
    }
};

#endif
