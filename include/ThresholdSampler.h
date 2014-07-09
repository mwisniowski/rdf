#ifndef THRESHOLD_SAMPLER_H
#define THRESHOLD_SAMPLER_H

#include <cvt/math/Math.h>

#include "Interfaces.h"
#include "DataRange.h"

using namespace cvt::Math;

template< typename D, typename F, typename S >
class ThresholdSampler
{ 
  private:
    const ITrainingContext< D, F, S >& context;
    const F                feature;
    const size_t           n;
    const DataRange< D >   range;
 
  public:
    ThresholdSampler( const ITrainingContext< D, F, S >& context, const F& f, const DataRange< D >& r ) :
      context( context ),
      feature( f ),
      range( r ),
      n( std::distance( r.begin(), r.end() ) )
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
      std::sort( range.begin(), range.end()(), *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        thresholds.push_back( feature( *( range.begin() + index ) ) );
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
      std::sort( range.begin(), range.end(), *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = cvt::Math::max( ceilf( n * i / static_cast<float>( size ) ) - 1, 0.0f );
        thresholds.push_back( feature( *( range.begin() + index ) ) );
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
      std::sort( range.begin(), range.end(), *this );

      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t a = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) ),
              b = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        float middle = ( feature( *( range.begin() + a ) ) + feature( *( range.begin() + b ) ) ) / 2.0f;
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
    bool operator()( const D& a, const D& b )
    {
      return context.lookup( feature, a ) < context.lookup( feature, b );
    }

  private:
    /**
     * @brief Finds minimum and maximum feature value in DataCollection
     *
     * @param min
     * @param max
     * @param range
     */
    void getMinMax( float& min, float& max, const DataRange< D >& range )
    {
      min = FLT_MAX;
      max = -min;

      typename DataRange< D >::const_iterator it = range.begin();
      for( ; it != range.end(); ++it )
      {
        float response = context.lookup( feature, *it );
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
