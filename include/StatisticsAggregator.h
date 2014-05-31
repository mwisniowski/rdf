#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include <map>
#include "IStatisticsAggregator.h"

class StatisticsAggregator// : public IStatisticsAggregator
{
  public:
    size_t n;

  private:
    // map< u_int, float > statistics;
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

    // StatisticsAggregator& operator=( const StatisticsAggregator& other )
    // {
    //   if( this != &other )
    //   {
    //     statistics = other.statistics;
    //     n = other.n;
    //   }
    //   
    //   return *this;
    // }

    virtual ~StatisticsAggregator() 
    {}

    void aggregate( const IDataPointRange& range )
    {
      IDataPointCollection::const_iterator it( range.start );
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

    // void aggregate( const DataPoint2f& point )
    // {
    //   map< u_int, float>::iterator it = statistics.find( point.output ),
    //     end = statistics.end();
    //   if( it == end )
    //   {
    //     it = statistics.insert( pair< u_int, float >( point.output, 0.0f ) ).first;
    //   }
    //   it->second++;
    //   n++;
    // }
    //
    // void aggregate( const IDataPointCollection& data )
    // {
    //   IDataPointCollection::const_iterator it = data.begin(),
    //     end = data.end();
    //   for( ; it != end; ++it )
    //   {
    //     aggregate( *it );
    //   }
    // }

    // size_t numClasses() const
    // {
    //   return n;
    // }

    // float probability( u_int class_label ) const
    // {
    //   return statistics[ class_label ] / n;
    // }

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
