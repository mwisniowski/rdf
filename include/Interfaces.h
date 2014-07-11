#ifndef INTERFACES_H
#define INTERFACES_H

#include <vector>
#include "TrainingParameters.h"

template< typename D >
class IFeature 
{
  public:
    size_t id;

    IFeature& operator=( const IFeature& other )
    {
      if( this != &other )
      {
        id = other.id;
      }
      return *this;
    }

    virtual float operator()( const D& point ) const =0;
};

template< typename D, typename F, typename S >
class ITrainingContext
{
  /**
   * Start implementing here
   */
  public:
    virtual S get_statistics() const =0;

    virtual float compute_information_gain( const S& parent_s,
        const S& left_s,
        const S& right_s ) const =0;

    virtual bool should_terminate( float information_gain ) const =0;

  protected:
    typedef std::vector< F > (*pool_init_fn)( size_t pool_size );

  /**
   * End implementing here
   */

  private:
    typedef std::vector< float >     row_type;
    typedef std::vector< row_type >  table_type;
    typedef std::vector< F >         pool_type;

  public:
    const TrainingParameters     params;
    const std::vector< F >       feature_pool;
    const std::vector< D >       data;
    const table_type             table;
    const size_t                 num_classes;

  public:
    ITrainingContext( const TrainingParameters& params,
        const std::vector< D >& data,
        pool_init_fn pool_init,
        size_t num_classes ) :
      params( params ),
      num_classes( num_classes ),
      data( data ),
      feature_pool( pool_init( params.pool_size ) ),
      table( create_table( feature_pool, data ) )
    {
    }

    ITrainingContext( const ITrainingContext& other ) :
      params( other.params ),
      feature_pool( other.feature_pool ),
      data( other.data ),
      table( other.table ),
      num_classes( other.num_classes )
    {}


  private:
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

    static table_type create_table( const std::vector< F >& feature_pool,
        const std::vector< D >& data )
    {
      table_type table( data.size() );

      for( size_t d = 0; d < data.size(); d++ )
      {
        for( size_t f = 0; f < feature_pool.size(); f++ )
        {
          table[ d ].push_back( feature_pool[ f ]( data[ d ] ) );
        }
      }

      return table;
    }

  public:
    float lookup( size_t data_idx, size_t feature_idx ) const
    {
      return table[ data_idx ][ feature_idx ];
    }

    void get_random_features( std::vector< size_t >& random_feature_idxs ) const
    {
      std::vector< size_t > feature_idxs = ascending_idxs( feature_pool.size() );
      random_shuffle( feature_idxs.begin(), feature_idxs.end() );

      random_feature_idxs.clear();
      for( size_t i = 0; i < params.no_candidate_features; i++ )
      {
        random_feature_idxs.push_back( feature_idxs[ i ] ); 
      }
    }

    std::vector< size_t > get_data_idxs() const
    {
      return ascending_idxs( data.size() );
    }
};

template< typename D, typename F, typename S >
class IStatistics 
{ 
  public:
    typedef ITrainingContext< D, F, S > ContextType;
    const ContextType& context;

    IStatistics( const ContextType& context ) :
      context( context )
    {}

    IStatistics( const IStatistics& other ) :
      context( other.context )
    {}

    virtual ~IStatistics()
    {}

    virtual S& operator+=( const std::vector< size_t >& data_idxs ) =0;

    virtual S& operator+=( const S& s ) =0;
};



#endif
