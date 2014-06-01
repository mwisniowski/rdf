#ifndef STATISTICS_AGGREGATOR_H
#define STATISTICS_AGGREGATOR_H

#include <vector>
#include <cvt/math/Math.h>

#include "DataCollection.h"

using namespace std;
class StatisticsAggregator
{
  public:
    size_t n;

  private:
    vector< float > statistics;


  public:
    StatisticsAggregator( size_t numClasses ) :
      n( 0 ),
      statistics( numClasses, 0.0f )
    {}

    StatisticsAggregator( const StatisticsAggregator& other ) :
      statistics( other.statistics ),
      n( other.n )
    {}

    virtual ~StatisticsAggregator() 
    {}

    void aggregate( const DataRange& range )
    {
      DataCollection::const_iterator it( range.start );
      for( ; it != range.end; ++it )
      {
        statistics[ it->output ]++;
        n++;
      }
    }

    void aggregate( const StatisticsAggregator& s )
    {
      //TODO ensure size
      vector< float >::iterator it = statistics.begin(), 
        end = statistics.end();
      vector< float >::const_iterator sit = s.statistics.begin();
      for( ; it != end; ++it, ++sit )
      {
        ( *it ) += ( *sit );
      }
      n += s.n;
    }

    pair< u_int, float > getMax() const
    {
      float maxValue = FLT_MIN;
      u_int maxC = 0;
      for( size_t i = 0; i < statistics.size(); i++ )
      {
        if( statistics[ i ] > maxValue )
        {
          maxValue = statistics[ i ];
          maxC = i;
        }
      }
      return pair< u_int, float >( maxC, maxValue / n );
    }

    float getEntropy() const
    {
      float entropy = 0.0f;

      for( size_t i = 0; i < statistics.size(); i++ )
      {
        if( float p_i = statistics[ i ] / n )
        {
          entropy += p_i * cvt::Math::log2( p_i );
        }
      }
      return -entropy;
    }

    friend ostream& operator<<( ostream& os, const StatisticsAggregator& s )
    {
      os << s.n << ": { ";
      for( size_t i = 0; i < s.statistics.size(); i++ )
      {
        os << "(" << i << "," << s.statistics[ i ] << ") ";
      }
      os << "}";

      return os;
    }


};

#endif
