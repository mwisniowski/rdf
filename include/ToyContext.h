#ifndef TOY_CONTEXT_H
#define TOY_CONTEXT_H

#include <cvt/math/Math.h>
#include <map>

#include "ToyCommon.h"
#include "ToyFeature.h"

#define POOL_SIZE 360

class ToyContext : public ITrainingContext< DataType, FeatureType, StatisticsType >
{
  private:
    typedef std::vector< float >        row_type;
    typedef std::vector< row_type >     table_type;
    typedef std::vector< FeatureType >  pool_type;

  public:
    const pool_type   featurePool;
    const table_type  table;
    const size_t      numClasses;

    ToyContext( const TrainingParameters p, 
        const DataRange< DataType >& range,
        size_t numClasses ) :
      ITrainingContext( p ),
      numClasses( numClasses ),
      featurePool( createFeaturePool() ),
      table( createTable( range ) )
    {
      srand( time( 0 ) );
    }

    ToyContext( const ToyContext& other ) :
      ITrainingContext( other.params ),
      numClasses( other.numClasses ),
      featurePool( other.featurePool ),
      table( other.table )
    {}

    virtual ~ToyContext()
    {
    }

  private:
    vector< FeatureType > createFeaturePool()
    {
      // TODO magic number
      size_t pool_size = POOL_SIZE;
      vector< FeatureType > features;
      features.reserve( pool_size );
      vector< float > rv;
      for( size_t id = 0; id < pool_size; id++ )
      {
        gaussianVector( rv, 2 );
        FeatureType f( rv );
        f.id = id;
        features.push_back( f );
      }
      return features;
    }

    table_type createTable( const DataRange< DataType >& range )
    {
      table_type table( std::distance( range.begin(), range.end() ) );

      table_type::iterator tit = table.begin();
      DataRange< DataType >::iterator dit = range.begin();
      size_t id = 0;
      for( ; dit != range.end(); ++dit, ++tit )
      {
        dit->id = id++;
        pool_type::const_iterator pit = featurePool.begin(),
          end = featurePool.end();
        for( ; pit != end; ++pit )
        {
          tit->push_back( ( *pit )( *dit ) );
        }
      }

      return table;
    }

    /**
     * @brief Generate a random gaussian distributed vector using the Marsaglia Polar Method
     *
     * @param gv
     * @param dimensions
     */
    void gaussianVector( vector< float >& gv, size_t dimensions )
    {
      gv.clear();
      for( size_t i = 0; i < dimensions; i+=2 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
        gv.push_back( v * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }

      if( dimensions % 2 == 1 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }
    }

  public:
    float lookup( const FeatureType& f, const DataType& p ) const
    {
      return table.at( p.id ).at( f.id );
    }

    /**
     * @brief Get random unit vectors by sampling an angle from the unit circle
     *
     * @param features
     */
    void getRandomFeatures( vector< FeatureType >& features ) const
    {
      vector< size_t > indices( featurePool.size() );
      for( size_t i = 0; i < indices.size(); i++ )
      {
        indices[ i ] = i;
      }
      random_shuffle( indices.begin(), indices.end() );

      features.clear();
      for( size_t i = 0; i < params.noCandidateFeatures; i++ )
      {
        features.push_back( featurePool[ indices[ i ] ] ); 
      }
    }

    StatisticsType getStatisticsAggregator() const
    {
      return StatisticsType( numClasses );
    }

    /**
     * @brief Compute information gain by subtracting the sum of weighted child-entropies
     * from parent entropy
     *
     * @param parent_s
     * @param left_s
     * @param right_s
     *
     * @return 
     */
    float computeInformationGain( const StatisticsType& parent_s,
        const StatisticsType& left_s,
        const StatisticsType& right_s ) const
    {

      float H_p = parent_s.getEntropy();
      float H_l = left_s.getEntropy();
      float H_r = right_s.getEntropy();

      float fraction = left_s.n / static_cast<float>( parent_s.n );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    /**
     * @brief Criterion if a leaf should be converted to split node
     *
     * @param information_gain
     *
     * @return 
     */
    bool shouldTerminate( float information_gain ) const
    {
      // TODO Magic number
      return information_gain < 0.01f;
    }
};

#endif
