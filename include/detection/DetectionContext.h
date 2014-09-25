#ifndef DETECTION_CONTEXT_H
#define DETECTION_CONTEXT_H

#include <cvt/gfx/Image.h>

#include "detection/DetectionCommon.h"
#include "detection/DetectionFeature.h"
#include "detection/DetectionStatistics.h"

class DetectionContext : public TrainingContextBase< InputType, StatisticsType, TestType, TreeType >
{
  private:
    typedef TrainingContextBase< InputType, StatisticsType, TestType, TreeType >
 super;

  public:
    DetectionContext( const TrainingParameters& params,
       const std::vector< cvt::Image >& images,
       const std::vector< std::vector< cvt::Recti > >& rois ) :
      super( params ),
      images_( images ),
      rois_( rois ),
      num_data_points_( images.size(), 0 )
    {
      for( size_t i = 0; i < images.size(); i++ )
      {
        num_data_points_[ i ] = ( images[ i ].width() - PATCH_SIZE ) * ( images[ i ].height() - PATCH_SIZE );
      }
    }

    virtual ~DetectionContext() 
    {}

    StatisticsType get_statistics() const
    {
      return StatisticsType();
    }

    StatisticsType get_root_statistics() const
    {
      StatisticsType s;

      //TODO only go over rois
      const size_t border = PATCH_SIZE / 2;
      for( size_t i = 0; i < images_.size(); i++ )
      {
        const cvt::Image& img = images_[ i ];
        for( size_t y = border; y < img.height() - border; y++ )
        {
          for( size_t x = border; x < img.width() - border; x++ )
          {
            s += get_output( i, x, y );
          }
        }
      }

      return s;
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
    float compute_information_gain( StatisticsType& parent_statistics,
        StatisticsType& left_statistics,
        StatisticsType& right_statistics ) const
    {
      if( parent_statistics.type() == 0 )
      {
        float H_p = parent_statistics.get_classification_entropy();
        float H_l = left_statistics.get_classification_entropy();
        float H_r = right_statistics.get_classification_entropy();

        float fraction = left_statistics.n() / static_cast<float>( parent_statistics.n() );

        return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
      } 
      else if( parent_statistics.type() == 1 )
      {
        float H_p = parent_statistics.get_regression_entropy();
        float H_l = left_statistics.get_regression_entropy();
        float H_r = right_statistics.get_regression_entropy();

        float fraction = left_statistics.n() / static_cast<float>( parent_statistics.n() );

        return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
      } 
      else
      {
        if( parent_statistics.probability( 0 ) < 0.05f )
        {
          parent_statistics.set_type( 1 );
        }
        else {
          parent_statistics.set_type( std::rand() % 2 );
        }
        return compute_information_gain( parent_statistics, left_statistics, right_statistics );
      }
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

    void fill_statistics( std::vector< StatisticsType* >& candidate_statistics,
        const std::vector< TestType >& random_tests,
        const std::vector< Path >& blacklist,
        const std::vector< Path >& paths ) const
    {
      const size_t border = PATCH_SIZE / 2;
      for( size_t i = 0; i < images_.size(); i++ )
      {
        size_t path_idx = 0;
        for( size_t j = 0; j < i; j++ )
        {
          path_idx += num_data_points_[ j ];
        }

        size_t counter = 0;
        const cvt::Image& img = images_[ i ];
        cvt::IMapScoped< const uint8_t > map( img );
        for( size_t y = border; y < img.height() - border; y++ )
        {
          for( size_t x = border; x < img.width() - border; x++ )
          {
            const Path& path = paths[ path_idx + counter ];
            counter++;
            if( path.is_blacklisted( blacklist ) )
            {
              continue;
            }

            const InputType in = { map, x, y };
            size_t idx = path.path();
            for( size_t j = 0; j < random_tests.size(); j++ )
            {
              size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
              bool result = random_tests[ j ]( in );
              if( result ) 
              {
                candidate_idx++;
              }

              candidate_statistics[ candidate_idx ]->operator+=( get_output( i, x, y ) );
            }
          }
        }
      }
    } 

    void update_paths( std::vector< Path >& paths,
         const std::vector< Path >& blacklist, 
         const TreeType& tree ) const
    {
      const size_t border = PATCH_SIZE / 2;
      for( size_t i = 0; i < images_.size(); i++ )
      {
        size_t path_idx = 0;
        for( size_t j = 0; j < i; j++ )
        {
          path_idx += num_data_points_[ j ];
        }

        size_t counter = 0;
        const cvt::Image& img = images_[ i ];
        cvt::IMapScoped< const uint8_t > map( img );
        for( size_t y = border; y < img.height() - border; y++ )
        {
          for( size_t x = border; x < img.width() - border; x++ )
          {
            Path& path = paths[ path_idx + counter ];
            counter++;
            if( !path.is_blacklisted( blacklist ) )
            {
              const InputType in = { map, x, y };
              path.add( tree.get_node( path )->test( in ) );
            }
          }
        }
      }
    }


  private:
    std::vector< cvt::Image > images_;
    std::vector< std::vector< cvt::Recti > > rois_;
    std::vector< size_t > num_data_points_;

    OutputType get_output( size_t image_idx, size_t x, size_t y ) const
    {
      const std::vector< cvt::Recti >& rects = rois_[ image_idx ];

      for( size_t r = 0; r < rects.size(); r++ )
      {
        const cvt::Recti& rect = rects[ r ];
        if( rect.contains( x, y ) )
        {
          cvt::Vector2i center( rect.x + ( rect.width / 2 ), rect.y + ( rect.height / 2 ) );
          return OutputType( 1, center - cvt::Vector2i( x, y ) );
        }
      }
      return OutputType( 0, cvt::Vector2i( 0, 0 ) );
    }
};

#endif
