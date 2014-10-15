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
        const std::vector< DataType >& data ):
      super( params ),
      data_( data )
    {}

    virtual ~DetectionContext() 
    {}

    StatisticsType get_statistics() const
    {
      return StatisticsType();
    }

    StatisticsType get_root_statistics() const
    {
      StatisticsType s;

      for( size_t i = 0; i < data_.size(); i++ )
      {
        s += data_[ i ].output();
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
      for( size_t i = 0; i < data_.size(); i++ )
      {
        const Path& path = paths[ i ];
        if( path.is_blacklisted( blacklist ) )
        {
          continue;
        }

        size_t idx = path.path();
        for( size_t j = 0; j < random_tests.size(); j++ )
        {
          size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
          bool result = random_tests[ j ]( data_[ i ].input() );
          if( result ) 
          {
            candidate_idx++;
          }

          StatisticsType& s = *candidate_statistics[ candidate_idx ];
          s += data_[ i ].output();
        }
      }
    } 

    void update_paths( std::vector< Path >& paths,
         const std::vector< Path >& blacklist, 
         const TreeType& tree ) const
    {
      for( size_t i = 0; i < data_.size(); i++ )
      {
        Path& path = paths[ i ];
        if( !path.is_blacklisted( blacklist ) )
        {
          path.add( tree.get_node( path )->test( data_[ i ].input() ) );
        }
      }
    }


  private:
    std::vector< DataType > data_;

};

#endif
