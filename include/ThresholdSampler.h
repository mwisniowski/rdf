#ifndef THRESHOLD_SAMPLER_H
#define THRESHOLD_SAMPLER_H

#include <cvt/math/Math.h>

#include "Feature.h"
#include "DataCollection.h"

using namespace cvt::Math;
class ThresholdSampler
{ 
  private:
    const Feature     feature;
    const size_t      n;
    const DataRange   range;
 
  public:
    ThresholdSampler( const Feature& f, const DataRange& r ) :
      feature( f ),
      range( r ),
      n( std::distance( r.start, r.end ) )
    {
    }

    /**
     * @brief Samples thresholds from the cumulative distribution of the data
     * DataCollection will be sorted
     *
     * @param thresholds
     * @param size
     */
    void cdf( vector< float >& thresholds, size_t size )
    {
      std::sort( range.start, range.end, *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        thresholds.push_back( feature( *( range.start + index ) ) );
      }
    }

    /**
     * @brief Sample thresholds are the q-quantiles of cdf.
     * DataCollection will be sorted
     *
     * @param thresholds
     * @param size
     */
    void quantiles( vector< float >& thresholds, size_t size )
    {
      std::sort( range.start, range.end, *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = cvt::Math::max( ceilf( n * i / static_cast<float>( size ) ) - 1, 0.0f );
        thresholds.push_back( feature( *( range.start + index ) ) );
      }
    }

    /**
     * @brief Samples thresholds uniformly between minimum and maximum
     *
     * @param thresholds
     * @param size
     */
    void uniform( vector< float >& thresholds, size_t size )
    {
      thresholds.clear();
      thresholds.reserve( size );
      float min, max;
      getMinMax( min, max, range );
      for( size_t i = 0; i < size; i++ )
      {
        thresholds.push_back( rand( min, max ) );
      }
    }

    /**
     * @brief Samples to values as in the cdf function and then uses their
     * mean as threshold.
     * DataCollection will be sorted
     *
     * @param thresholds
     * @param size
     */
    void mean( vector< float >& thresholds, size_t size )
    {
      std::sort( range.start, range.end, *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t a = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) ),
              b = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        float middle = ( feature( *( range.start + a ) ) + feature( *( range.start + b ) ) ) / 2.0f;
        thresholds.push_back( middle );
      }
    }

    /**
     * @brief Comparator for sorting DataCollection
     *
     * @param a
     * @param b
     *
     * @return 
     */
    bool operator()( const DataPoint2f& a, const DataPoint2f& b )
    {
      return feature( a ) < feature( b );
    }

  private:
    /**
     * @brief Finds minimum and maximum feature value in DataCollection
     *
     * @param min
     * @param max
     * @param range
     */
    void getMinMax( float& min, float& max, const DataRange& range )
    {
      min = FLT_MAX;
      max = FLT_MIN;
      
      DataCollection::const_iterator it = range.start;
      for( ; it != range.end; ++it )
      {
        float response = feature( *it );
        if( response < min )
        {
          min = response;
        }
        if( response > max )
        {
          max = response;
        }
      }
    }
};

#endif
