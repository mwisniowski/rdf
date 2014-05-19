#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include <map>
#include "IStatisticsAggregator.h"

class StatisticsAggregator //: public IStatisticsAggregator
{
  public:
    StatisticsAggregator() 
    {}

    StatisticsAggregator( const StatisticsAggregator& other ) :
      _statistics( other._statistics ),
      _n( other._n )
    {}

    StatisticsAggregator& operator=( const StatisticsAggregator& other )
    {
      if( this != &other )
      {
        _statistics = other._statistics;
        _n = other._n;
      }
      
      return *this;
    }

    virtual ~StatisticsAggregator() 
    {}

    void aggregate( DataPoint2f& point )
    {
      _statistics.at( point.output() )++;
      _n++;
    }

    void aggregate( IDataPointCollection& data )
    {
      IDataPointCollection::iterator it = data.begin(),
        end = data.end();
      for( ; it != end; ++it )
      {
        aggregate( *it );
      }
    }

    size_t numClasses() const
    {
      return _n;
    }

    float probability( size_t class_label ) const
    {
      return _statistics.at( class_label ) / _n;
    }

    u_int maxClass() const
    {
      map< u_int, float>::const_iterator it = _statistics.begin(),
        end = _statistics.end();

      pair< u_int, float > max;
      for( ; it != end; ++it )
      {
        if ( it->second > max.second )
        {
          max = *it;
        }
      }

      return max.first;
    }

    float entropy()
    {
      if( _entropy.get() == NULL )
      {
        _entropy = auto_ptr< float >( new float( 0.0f ) );
        map< u_int, float >::const_iterator it = _statistics.begin(),
          end = _statistics.end();
        for( ; it != end; ++it )
        {
          float p_c = it->second / _n;
          //TODO log2
          *_entropy += p_c * cvt::Math::log( p_c );
        }
      }
      return *_entropy;
    }

  private:
    map< u_int, float > _statistics;
    size_t _n;
    auto_ptr< float > _entropy;

};

#endif
