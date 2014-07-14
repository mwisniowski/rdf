#ifndef IMAGE_CONTEXT_H
#define IMAGE_CONTEXT_H

#include <cvt/gfx/Image.h>

#include "ImageCommon.h"
#include "ImageFeature.h"

class ImageContext : public TrainingContextBase< DataType, FeatureType, StatisticsType >
{
  private:
    typedef TrainingContextBase< DataType, FeatureType, StatisticsType > super;

  public:
    ImageContext( const TrainingParameters& params, 
                  const std::vector< DataType >& data, 
                  size_t num_classes ) :
      super( params, data, num_classes )
    {
    }

    virtual ~ImageContext() 
    {}

    StatisticsType get_statistics()
    {
      return StatisticsType( *this );
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
    float compute_information_gain( const StatisticsType& parent_statistics,
        const StatisticsType& left_statistics,
        const StatisticsType& right_statistics ) const
    {
      float H_p = parent_statistics.get_entropy();
      float H_l = left_statistics.get_entropy();
      float H_r = right_statistics.get_entropy();

      float fraction = left_statistics.n() / static_cast<float>( parent_statistics.n() );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    /**
     * @brief Criterion if a leaf should be converted to split node
     *
     * @param information_gain
     *
     * @return 
     */
    bool should_terminate( float information_gain ) const
    {
      // TODO Magic number
      return information_gain < 0.01f;
    }

  private:
    static std::vector< FeatureType > pool_init( size_t pool_size )
    {
      std::vector< FeatureType > features;
      features.reserve( pool_size );
      for( size_t i = 0; i < pool_size; i++ )
      {
        features.push_back( FeatureType::get_random_feature() );
      }
      return features;
    }
};

#endif
