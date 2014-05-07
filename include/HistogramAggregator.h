#ifndef CVT_RDF_HISTOGRAM_AGGREGATOR_H
#define CVT_RDF_HISTOGRAM_AGGREGATOR_H

#include "IStatisticsAggregator.h"
#include <cvt/math/Math.h>

class HistogramAggregator : public IStatisticsAggregator
{
  public:
    HistogramAggregator() {}
    virtual ~HistogramAggregator() {}

    void aggregate( DataPoint2f& point )
    {
      _histogram[ point.output() ]++;
      _total_points++;
    }

    float entropy()
    {
      float sum = 0.0f;
      for( size_t i = 0; i < _histogram.size(); i++ )
      {
        float probability = _histogram[ i ] / _total_points;
        sum += probability * cvt::Math::log2( probability );
      }
      return -sum;
    }

  private:
    vector< size_t > _histogram;
    size_t _total_points;
};

#endif
