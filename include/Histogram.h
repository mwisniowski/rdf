#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include "DataRange.h"
#include "Interfaces.h"

typedef DataPoint< float, u_int, 2 > DataPoint2f;

class Histogram: public IStatistics< DataPoint2f, Histogram >
{
  public:
    size_t n;

  private:
    vector< size_t > histogram;


  public:
    Histogram() :
      n( 0 )
    {}

    Histogram( const Histogram& other ) :
      histogram( other.histogram ),
      n( other.n )
    {}

    virtual ~Histogram() 
    {}

    /**
     * @brief Adds DataPoints from range to histogram
     *
     * @param range
     */
    void aggregate( const DataRange< DataPoint2f >& range )
    {
      DataRange< DataPoint2f >::const_iterator it( range.begin() );
      for( ; it != range.end(); ++it )
      {
        if( numClasses() <= it->output )
        {
          histogram.resize( it->output + 1, 0.0f );
        }
        histogram[ it->output ]++;
        n++;
      }
    }

    /**
     * @brief Adds per-class values from another histogram
     *
     * @param s
     */
    void aggregate( const Histogram& s )
    {
      if( s.numClasses() > numClasses() )
      {
        histogram.resize( s.numClasses(), 0.0f );
      }
      vector< size_t >::iterator it = histogram.begin();
      vector< size_t >::const_iterator sit = s.histogram.begin(),
        send = s.histogram.end();
      for( ; sit != send; ++it, ++sit )
      {
        ( *it ) += ( *sit );
      }
      n += s.n;
    }

    /**
     * @brief Returns mode of histogram with the corresponding
     * empirical class probability
     *
     * @return 
     */
    pair< u_int, float > getMode() const
    {
      float maxValue = FLT_MIN;
      u_int maxC = 0;
      for( size_t i = 0; i < numClasses(); i++ )
      {
        if( histogram[ i ] > maxValue )
        {
          maxValue = histogram[ i ];
          maxC = i;
        }
      }
      return pair< u_int, float >( maxC, maxValue / n );
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float getEntropy() const
    {
      float entropy = 0.0f;
      for( size_t i = 0; i < numClasses(); i++ )
      {
        if( float p_i = static_cast<float>( histogram[ i ] ) / n )
        {
          entropy += p_i * cvt::Math::log2( p_i );
        }
      }
      return -entropy;
    }

    float probability( u_int class_index ) const
    {
      if( class_index >= numClasses() )
      {
        return 0.0f;
      }
      return static_cast<float>( histogram[ class_index ] ) / n;
    }

    size_t numClasses() const
    {
      return histogram.size();
    }

    friend ostream& operator<<( ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      for( size_t i = 0; i < s.numClasses(); i++ )
      {
        os << "(" << i << "," << s.histogram[ i ] << ") ";
      }
      os << "}";

      return os;
    }


};

#endif
