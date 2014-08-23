#ifndef TOY_CONTEXT_H
#define TOY_CONTEXT_H

#include <cvt/math/Math.h>
#include <map>

#include "ToyCommon.h"
#include "ToyFeature.h"

class ToyContext : public TrainingContextBase< DataType, FeatureType, StatisticsType >
{
  private:
    typedef TrainingContextBase< DataType, FeatureType, StatisticsType > super;

  public:
    ToyContext( const TrainingParameters& params,
        const std::vector< DataType >& data,
        size_t num_classes ) :
      super( params, data ),
      num_classes_( num_classes )
    {}

    ToyContext( const ToyContext& other ) :
      super( other ),
      num_classes_( other.num_classes_ )
    {}

    virtual ~ToyContext()
    {}

    StatisticsType get_statistics() const
    {
      return StatisticsType( num_classes_ );
    }

    StatisticsType get_statistics( const std::vector< size_t >& data_idxs ) const
    {
      StatisticsType s( num_classes_ );
      for( size_t i = 0; i < data_idxs.size(); ++i )
      {
        const DataType& d = data_point( data_idxs[ i ] );
        s += d;
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
    size_t num_classes_;
};

#endif
