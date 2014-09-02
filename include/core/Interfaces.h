#ifndef INTERFACES_H
#define INTERFACES_H

#include <vector>
#include <iostream>
#include "core/DataPoint.h"
#include "core/TrainingParameters.h"

template< typename I, typename O >
class FeatureBase
{
  public:
    virtual float operator()( const DataPoint< I, O >& point ) const =0;

    /**
     * IMPLEMENT THIS!
     * static FeatureBase get_random_feature();
     */
};

template< typename I, typename O, typename F, typename S >
class TrainingContextBase
{
  /**
   * Start implementing here
   */

  public:
    virtual S get_statistics() const =0;

    virtual S get_statistics( const std::vector< size_t >& data_idxs ) const =0;

    virtual float compute_information_gain( S& parent_s,
        S& left_s,
        S& right_s ) const =0;

    virtual bool should_terminate( float information_gain ) const =0;

  /**
   * End implementing here
   */

  private:
    // typedef std::vector< float > row_type;
    // typedef F (*feature_generator_fn)();

  public:
    TrainingContextBase( const TrainingParameters& params ) :
      params_( params ),
      features_( init_features( params.pool_size ) ),
      table_( features_.size(), std::vector< float >() )
    {}

    // TrainingContextBase( const TrainingParameters& params,
    //     const std::vector< DataPoint< I, O > >& data ) :
    //   params_( params ),
    //   outputs_( outputs ),
    //   features_( init_features( params.pool_size ) ),
    //   table_( init_table( features_, data ) )
    // {}

    TrainingContextBase( const TrainingContextBase& other ) :
      params_( other.params_ ),
      features_( other.features_ ),
      outputs_( other.outputs_ ),
      table_( other.table_ )
    {}

  public:
    TrainingContextBase& operator+=( const DataPoint< I, O >& d )
    {
      for( size_t f = 0; f < features_.size(); f++ )
      {
        table_[ f ].push_back( features_[ f ]( d ) );
      }
      outputs_.push_back( d.output() );

      return *this;
    }

    float lookup( size_t data_idx, size_t feature_idx ) const
    {
      return table_[ feature_idx ][ data_idx ];
    }

    void get_random_features( std::vector< size_t >& random_feature_idxs ) const
    {
      std::vector< size_t > feature_idxs = ascending_idxs( features_.size() );
      random_shuffle( feature_idxs.begin(), feature_idxs.end() );

      random_feature_idxs.clear();
      for( size_t i = 0; i < params_.no_candidate_features; i++ )
      {
        random_feature_idxs.push_back( feature_idxs[ i ] ); 
      }
    }

    std::vector< size_t > get_data_idxs() const
    {
      return ascending_idxs( outputs_.size() );
    }

    const TrainingParameters& params() const
    {
      return params_;
    }

    const F& feature( size_t idx ) const
    {
      return features_[ idx ];
    }

    // const DataPoint< I, O >& data_point( size_t idx ) const
    // {
    //   return data_[ idx ];
    // }
    
    const O& output( size_t idx ) const
    {
      return outputs_[ idx ];
    }

  private:
    TrainingParameters                   params_;
    std::vector< F >                     features_;
    // std::vector< DataPoint< I, O > >                     data_;
    std::vector< O > outputs_;
    std::vector< std::vector< float > >  table_;

    static std::vector< size_t > ascending_idxs( size_t size )
    {
      std::vector< size_t > idxs;
      idxs.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        idxs.push_back( i );
      }

      return idxs;
    }

    static std::vector< F > init_features( size_t size )
    {
      std::vector< F > features;
      for( size_t i = 0; i < size; i++ )
      {
        features.push_back( F::get_random_feature() );
      }
      return features;
    }

    static std::vector< std::vector< float > > init_table( const std::vector< F >& features,
        const std::vector< DataPoint< I, O > >& data )
    {
      std::vector< std::vector< float > > table( features.size(), std::vector< float >( data.size() ) );

      for( size_t d = 0; d < data.size(); d++ )
      {
        for( size_t f = 0; f < features.size(); f++ )
        {
          table[ f ][ d ] = features[ f ]( data[ d ] );
        }
      }

      return table;
    }

};

template< typename I, typename O, typename F, typename S >
class StatisticsBase 
{ 
  public:
    virtual S& operator+=( const S& s ) =0;
    virtual S& operator+=( const O& o ) =0;
};



#endif
