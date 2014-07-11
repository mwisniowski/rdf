#ifndef INTERFACES_H
#define INTERFACES_H

#include <vector>
#include "TrainingParameters.h"
#include "DataRange.h"

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
    typedef vector< F > (*pool_init_fn)( size_t pool_size );

  /**
   * End implementing here
   */

  private:
    typedef std::vector< float >     row_type;
    typedef std::vector< row_type >  table_type;
    typedef std::vector< F >         pool_type;

  public:
    const TrainingParameters  params;
    const vector< F >         feature_pool;
    const vector< D >         data;
    const table_type          table;
    const size_t              num_classes;
    const vector< size_t >    data_idxs;

  public:
    ITrainingContext( const TrainingParameters& params,
        const vector< D >& data,
        pool_init_fn pool_init,
        size_t num_classes,
        size_t pool_size ) :
      params( params ),
      num_classes( num_classes ),
      data( data ),
      data_idxs( ascending_idxs( data.size() ) ),
      feature_pool( pool_init( pool_size ) ),
      table( create_table( feature_pool, data ) )
    {
      srand( time( 0 ) );
    }

    ITrainingContext( const ITrainingContext& other ) :
      num_classes( other.num_classes ),
      feature_pool( other.feature_pool ),
      table( other.table )
    {}


  private:
    static vector< size_t > ascending_idxs( size_t size )
    {
      vector< size_t > idxs;
      idxs.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        idxs.push_back( i );
      }

      return idxs;
    }

    static table_type create_table( const vector< F >& feature_pool, 
        const vector< D >& data )
    {
      table_type table( std::distance( data.begin(), data.end() ) );

      table_type::iterator tit = table.begin();
      typename DataRange< D >::const_iterator dit = data.begin();
      size_t id = 0;
      for( ; dit != data.end(); ++dit, ++tit )
      {
        typename pool_type::const_iterator pit = feature_pool.begin(),
          end = feature_pool.end();
        for( ; pit != end; ++pit )
        {
          tit->push_back( ( *pit )( *dit ) );
        }
      }

      return table;
    }

  public:
    float lookup( size_t data_idx, size_t feature_idx ) const
    {
      return table[ data_idx ][ feature_idx ];
    }

    void get_random_features( vector< size_t >& random_feature_idxs ) const
    {
      vector< size_t > feature_idxs = ascending_idxs( feature_pool.size() );
      random_shuffle( feature_idxs.begin(), feature_idxs.end() );

      random_feature_idxs.clear();
      for( size_t i = 0; i < params.no_candidate_features; i++ )
      {
        random_feature_idxs.push_back( feature_idxs[ i ] ); 
      }
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

    virtual S& operator+=( const vector< size_t >& data_idxs ) =0;

    virtual S& operator+=( const S& s ) =0;
};



#endif
