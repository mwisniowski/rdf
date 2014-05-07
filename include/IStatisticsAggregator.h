#ifndef CVT_RDF_ISTATISTICS_AGGREGATOR
#define CVT_RDF_ISTATISTICS_AGGREGATOR

#include <vector>
#include "DataPoint.h"

using namespace std;

class IStatisticsAggregator 
{
  public:
    virtual void aggregate( const DataPoint2f& point ) =0;
    virtual size_t numClasses() const =0;
    virtual float probability( size_t class_label ) const =0;
};

#endif
