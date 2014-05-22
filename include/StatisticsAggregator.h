#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include <map>
#include "IStatisticsAggregator.h"

class StatisticsAggregator //: public IStatisticsAggregator
{
  public:
    StatisticsAggregator() :
      n( 0 )
    {}

    StatisticsAggregator( const StatisticsAggregator& other ) :
      statistics( other.statistics ),
      n( other.n )
    {}

    StatisticsAggregator& operator=( const StatisticsAggregator& other )
    {
      if( this != &other )
      {
        statistics = other.statistics;
        n = other.n;
      }
      
      return *this;
    }

    virtual ~StatisticsAggregator() 
    {}

    void aggregate( DataPoint2f& point )
    {
      map< u_int, float >::iterator it =
        statistics.insert( pair< u_int, float >( point.output, 0.0f ) ).first;
      it->second++;
      n++;
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
      return n;
    }

    float probability( size_t class_label ) const
    {
      map< u_int, float>::const_iterator it = statistics.find( class_label ),
        end;
      if( it != end )
      {
        return it->second / n;
      } else {
        return 0.0f;
      }
    }

    u_int maxClass() const
    {
      map< u_int, float>::const_iterator it = statistics.begin(),
        end = statistics.end();

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

    float getEntropy()
    {
      if( entropy.get() == NULL )
      {
        entropy = auto_ptr< float >( new float( 0.0f ) );

        map< u_int, float >::const_iterator it = statistics.begin(),
          end = statistics.end();
        for( ; it != end; ++it )
        {
          float p_c = it->second / n;
          *entropy += p_c * cvt::Math::log2( p_c );
        }
        *entropy *= -1;
      }
      return *entropy;
    }

    friend ostream& operator<<( ostream& os, const StatisticsAggregator& s )
    {
      os << s.n << ": { ";
      map< u_int, float >::const_iterator it = s.statistics.begin(),
        end = s.statistics.end();
      for( ; it != end; ++it )
      {
        os << "(" << it->first << "," << it->second << ") ";
      }
      os << "}";

      return os;
    }

  private:
    map< u_int, float > statistics;
    size_t n;
    auto_ptr< float > entropy;

};

#endif
