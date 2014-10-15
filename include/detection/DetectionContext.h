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
       const std::vector< std::vector< cvt::Image > >& images,
       const std::vector< std::vector< cvt::Recti > >& rois ) :
      super( params ),
      images_( images ),
      rois_( rois )
    {
      const int border = PATCH_SIZE / 2;
      for( size_t i = 0; i < images.size(); i++ )
      {
        std::vector< std::pair< size_t, size_t > > sample_i;
        size_t pos = 0;
        while( pos < SAMPLE_SIZE / 2 )
        {
          size_t x = cvt::Math::rand( border, images[ i ][ 0 ].width() - border );
          size_t y = cvt::Math::rand( border, images[ i ][ 0 ].height() - border );
          OutputType o = get_output( i, x, y );
          if( o.first == 1 )
          {
            pos++;
            sample_i.push_back( std::pair< size_t, size_t >( x, y ) );
          }
        }
        size_t neg = 0;
        while( neg < SAMPLE_SIZE / 2 )
        {
          size_t x = cvt::Math::rand( border, images[ i ][ 0 ].width() - border );
          size_t y = cvt::Math::rand( border, images[ i ][ 0 ].height() - border );
          OutputType o = get_output( i, x, y );
          if( o.first == 0 )
          {
            neg++;
            sample_i.push_back( std::pair< size_t, size_t >( x, y ) );
          }
        }
        samples_.push_back( sample_i );
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

      const size_t border = PATCH_SIZE / 2;
      for( size_t i = 0; i < images_.size(); i++ )
      {
        const std::vector< std::pair< size_t, size_t > >& sample = samples_[ i ];
        for( size_t j = 0; j < SAMPLE_SIZE; j++ )
        {
          s += get_output( i, sample[ j ].first, sample[ j ].second );
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
      if( parent_statistics.type() == StatisticsType::CLASSIFICATION )
      {
        float H_p = parent_statistics.get_classification_entropy();
        float H_l = left_statistics.get_classification_entropy();
        float H_r = right_statistics.get_classification_entropy();

        float fraction = left_statistics.n() / static_cast<float>( parent_statistics.n() );

        return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
      } 
      else if( parent_statistics.type() == StatisticsType::REGRESSION )
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
          parent_statistics.set_type( StatisticsType::REGRESSION );
        }
        else {
          bool r = std::rand() % 2;
          if( r )
          {
            parent_statistics.set_type( StatisticsType::CLASSIFICATION );
          }
          else
          {
            parent_statistics.set_type( StatisticsType::REGRESSION );
          }
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
        const std::vector< cvt::Image >& channels = images_[ i ];
        const std::vector< std::pair< size_t, size_t > >& sample = samples_[ i ];

        for( size_t s = 0; s < SAMPLE_SIZE; s++ )
        {
          const Path& path = paths[ i * SAMPLE_SIZE + s ];
          if( path.is_blacklisted( blacklist ) )
          {
            continue;
          }
          const InputType in = { channels, sample[ s ].first, sample[ s ].second };

          size_t idx = path.path();
          for( size_t j = 0; j < random_tests.size(); j++ )
          {
            size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
            bool result = random_tests[ j ]( in );
            if( result ) 
            {
              candidate_idx++;
            }

            OutputType o = get_output( i, sample[ s ].first, sample[ s ].second );
            StatisticsType& s = *candidate_statistics[ candidate_idx ];
            s += o;
            // candidate_statistics[ candidate_idx ]->operator+=( o );
          }
        }
      }
    } 

    void update_paths( std::vector< Path >& paths,
         const std::vector< Path >& blacklist, 
         const TreeType& tree ) const
    {
      for( size_t i = 0; i < images_.size(); i++ )
      {
        const std::vector< cvt::Image >& channels = images_[ i ];
        const std::vector< std::pair< size_t, size_t > >& sample = samples_[ i ];

        for( size_t j = 0; j < SAMPLE_SIZE; j++ )
        {
          Path& path = paths[ i * SAMPLE_SIZE + j ];
          if( !path.is_blacklisted( blacklist ) )
          {
            const InputType in = { channels, sample[ j ].first, sample[ j ].second };
            path.add( tree.get_node( path )->test( in ) );
          }
        }
      }
    }


  private:
    std::vector< std::vector< cvt::Image > > images_;
    std::vector< std::vector< cvt::Recti > > rois_;
    std::vector< std::vector< std::pair< size_t, size_t > > > samples_;

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
