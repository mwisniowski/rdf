#ifndef THRESHOLD_SAMPLER_H
#define THRESHOLD_SAMPLER_H

#include <cvt/math/Math.h>

#include "Interfaces.h"

using namespace cvt::Math;

template< typename D, typename F, typename S >
class ThresholdSampler
{ 
  private:
    const ITrainingContext< D, F, S >&  context;
    const size_t                        feature_idx;
    // const size_t                        n;
    const vector< size_t >              data_idxs;
 
  public:
    ThresholdSampler( const ITrainingContext< D, F, S >& context, size_t feature_idx, const vector< size_t >& data_idxs ) :
      context( context ),
      feature_idx( feature_idx ),
      data_idxs( data_idxs )
      // n( data_idxs.size() )
    {
    }

    // /**
    //  * @brief Samples thresholds from the cumulative distribution of the data
    //  * DataCollection will be sorted
    //  *
    //  * @param thresholds
    //  * @param size
    //  */
    // void cdf( vector< float >& thresholds, size_t size )
    // {
    //   std::sort( range.begin(), range.end()(), *this );
    //
    //   thresholds.clear();
    //   thresholds.reserve( size );
    //   for( size_t i = 0; i < size; i++ )
    //   {
    //     size_t index = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
    //     thresholds.push_back( feature( *( range.begin() + index ) ) );
    //   }
    // }

    /**
     * @brief Sample thresholds are the q-quantiles of cdf.
     * DataCollection will be sorted
     *
     * @param thresholds
     * @param size
     */
    // void quantiles( vector< float >& thresholds, size_t size )
    // {
    //   std::sort( range.begin(), range.end(), *this );
    //
    //   thresholds.clear();
    //   thresholds.reserve( size );
    //   for( size_t i = 0; i < size; i++ )
    //   {
    //     size_t index = cvt::Math::max( ceilf( n * i / static_cast<float>( size ) ) - 1, 0.0f );
    //     thresholds.push_back( feature( *( range.begin() + index ) ) );
    //   }
    // }

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
      get_min_max( min, max );
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
    // void mean( vector< float >& thresholds, size_t size )
    // {
    //   std::sort( range.begin(), range.end(), *this );
    //
    //   thresholds.clear();
    //   thresholds.reserve( size );
    //   for( size_t i = 0; i < size; i++ )
    //   {
    //     size_t a = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) ),
    //           b = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
    //     float middle = ( feature( *( range.begin() + a ) ) + feature( *( range.begin() + b ) ) ) / 2.0f;
    //     thresholds.push_back( middle );
    //   }
    // }

    /**
     * @brief Comparator for sorting DataCollection
     *
     * @param a
     * @param b
     *
     * @return 
     */
    // bool operator()( const size_t a_data_idx, const size_t b_data_idx )
    // {
    //   return context.lookup( a_data_idx, feature_idx ) < context.lookup( b_data_idx, feature_idx );
    // }

  private:
    /**
     * @brief Finds minimum and maximum feature value in DataCollection
     *
     * @param min
     * @param max
     * @param range
     */
    void get_min_max( float& min, float& max ) const
    {
      min = FLT_MAX;
      max = -min;

      for( size_t i = 0; i < data_idxs.size(); i++ )
      {
        float response = context.lookup( data_idxs[ i ], feature_idx );
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
