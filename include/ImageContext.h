#ifndef IMAGE_CONTEXT_H
#define IMAGE_CONTEXT_H

#include <cvt/gfx/Image.h>

#include "ImageCommon.h"
#include "ImageFeature.h"

#define POOL_SIZE 1000
#define CHANNELS 3

class ImageContext : public ITrainingContext< DataType, FeatureType, StatisticsType >
{
  private:
    typedef ITrainingContext< DataType, FeatureType, StatisticsType >  super;
    typedef std::vector< float > row_type;
    typedef std::vector< row_type > table_type;
    typedef std::vector< FeatureType > pool_type;

  public:
    const vector< FeatureType >  featurePool;
    const table_type             table;

    ImageContext( const TrainingParameters& params, const DataRange< DataType >& range ) :
      super( params ),
      featurePool( createFeaturePool() ),
      table( createTable( range ) )
    {
      srand( time( 0 ) );
    }

    ImageContext( const ImageContext& other ) :
      super( other.params ),
      featurePool( other.featurePool ),
      table( other.table )
    {}

    virtual ~ImageContext() 
    {}

  private:
    vector< FeatureType > createFeaturePool()
    {
      // TODO magic number
      size_t pool_size = POOL_SIZE;
      vector< FeatureType > features;
      features.reserve( pool_size );
      vector< float > rv;
      for( size_t i = 0; i < pool_size; i++ )
      {
        cvt::Point2f p1( cvt::Math::rand( 0.0f, 1.0f ), cvt::Math::rand( 0.0f, 1.0f ) );
        cvt::Point2f p2( cvt::Math::rand( 0.0f, 1.0f ), cvt::Math::rand( 0.0f, 1.0f ) );
        size_t channel = cvt::Math::rand( 0, CHANNELS ) + 0.5f;
        features.push_back( FeatureType( p1, p2, channel ) );
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

  public:
    float lookup( const FeatureType& f, const DataType& p ) const
    {
      return table.at( f.id ).at( p.id );
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
      return StatisticsType();
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
