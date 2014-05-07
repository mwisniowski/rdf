#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <map>
#include "IStatisticsAggregator.h"

class StatisticsAggregator : public IStatisticsAggregator
{
  public:
    StatisticsAggregator() 
    {}

    virtual ~StatisticsAggregator() 
    {}

    void aggregate( const DataPoint2f& point )
    {
      _statistics.at( point.output() )++;
      _n++;
    }

    size_t numClasses() const
    {
      return _n;
    }

    float probability( size_t class_label ) const
    {
      return _statistics.at( class_label ) / _n;
    }

  private:
    map< u_int, float > _statistics;
    size_t _n;

};

#endif
