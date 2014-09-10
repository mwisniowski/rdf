#ifndef RDF_INTERFACES_H
#define RDF_INTERFACES_H

#include <vector>
#include <iostream>
#include <bitset>
#include <cvt/math/Math.h>

#include "core/DataPoint.h"
#include "core/TrainingParameters.h"

template< typename I, typename O, typename F >
class Test;

template< typename I, typename O >
class FeatureBase
{
  public:
    virtual float operator()( const std::vector< I >& input ) const =0;
};

template< typename I, typename O, typename F >
class TestSamplerBase
{
  public:
    virtual void sample( std::vector< Test< I, O, F > >& tests, size_t num_tests ) const =0;
};

template< typename O, typename S >
class StatisticsBase 
{ 
  public:
    virtual S& operator+=( const S& s ) =0;
    virtual S& operator+=( const O& o ) =0;
    virtual std::pair< O, float > predict() const =0;
};

template< typename I, typename O, typename S >
class TrainingContextBase
{
  /**
   * Start implementing here
   */

  public:
    virtual S get_statistics() const =0;

    virtual S get_statistics( const std::vector< DataPoint< I, O > >& data ) const =0;

    virtual float compute_information_gain( S& parent_s,
        S& left_s,
        S& right_s ) const =0;

    virtual bool should_terminate( float information_gain ) const =0;

  /**
   * End implementing here
   */

  public:
    TrainingContextBase( const TrainingParameters& params ) :
      params_( params )
    {}

    TrainingContextBase( const TrainingContextBase& other ) :
      params_( other.params_ )
    {}

    const TrainingParameters& params() const
    {
      return params_;
    }

  private:
    TrainingParameters   params_;

    // class ThresholdSampler
    // { 
    //   public:
    //     ThresholdSampler( size_t feature_idx, const std::vector< size_t >& data_idxs ) :
    //       feature_idx_( feature_idx ),
    //       data_idxs_( data_idxs )
    //     {}
    //
    //     /**
    //     * @brief Samples thresholds uniformly between minimum and maximum
    //     *
    //     * @param thresholds
    //     * @param size
    //     */
    //     void uniform( std::vector< float >& thresholds, size_t size, const TrainingContextBase< I, O, F, S >& context ) const
    //     {
    //       thresholds.clear();
    //       thresholds.reserve( size );
    //       float min, max;
    //       get_min_max( min, max, context );
    //       for( size_t i = 0; i < size; i++ )
    //       {
    //         thresholds.push_back( cvt::Math::rand( min, max ) );
    //       }
    //     }
    //
    //   private:
    //     const size_t                  feature_idx_;
    //     const std::vector< size_t >   data_idxs_;
    //
    //     /**
    //     * @brief Finds minimum and maximum feature value in DataCollection
    //     *
    //     * @param min
    //     * @param max
    //     * @param range
    //     */
    //     void get_min_max( float& min, float& max, const TrainingContextBase< I, O, F, S >& context ) const
    //     {
    //       min = FLT_MAX;
    //       max = -min;
    //
    //       for( size_t i = 0; i < data_idxs_.size(); i++ )
    //       {
    //         float response = context.lookup( data_idxs_[ i ], feature_idx_ );
    //         if( response < min )
    //         {
    //           min = response;
    //         }
    //         if( response > max )
    //         {
    //           max = response;
    //         }
    //       }
    //     }
    // };

};

#endif
