#ifndef DETECTION_CONTEXT_H
#define DETECTION_CONTEXT_H

#include <cvt/gfx/Image.h>

#include "detection/DetectionCommon.h"
#include "detection/DetectionFeature.h"
#include "detection/DetectionStatistics.h"

class DetectionContext : public TrainingContextBase< InputType, OutputType, StatisticsType >
{
  private:
    typedef TrainingContextBase< InputType, OutputType, StatisticsType > super;

  public:
    DetectionContext( const TrainingParameters& params ) :
      super( params )
    {}

    virtual ~DetectionContext() 
    {}

    StatisticsType get_statistics() const
    {
      return StatisticsType();
    }

    StatisticsType get_statistics( const std::vector< DataPoint< InputType, OutputType > >& data ) const
    {
      StatisticsType s;
      for( size_t i = 0; i < data.size(); ++i )
      {
        s += data[ i ].output();
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
};

#endif
