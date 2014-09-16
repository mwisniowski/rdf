#ifndef IMAGE_CONTEXT_H
#define IMAGE_CONTEXT_H

#include <cvt/gfx/Image.h>

#include "classification/HogCommon.h"
#include "classification/HogFeature.h"

class HogContext : public TrainingContextBase< InputType, StatisticsType, TestType, TreeType >
{
  private:
    typedef TrainingContextBase< InputType, StatisticsType, TestType, TreeType > super;

  public:
    HogContext( const TrainingParameters& params, 
        std::vector< DataType >& data,
        size_t num_classes ) :
      super( params ),
      data_( data ),
      num_classes_( num_classes )
    {}

    virtual ~HogContext() 
    {}

    StatisticsType get_statistics() const
    {
      return StatisticsType( num_classes_ );
    }

    StatisticsType get_root_statistics() const
    {
      StatisticsType s( num_classes_ );
      for( size_t i = 0; i < data_.size(); ++i )
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

    size_t num_data_points() const
    {
      return data_.size();
    }

    void fill_statistics( std::vector< StatisticsType* >& candidate_statistics,
        const std::vector< TestType >& random_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< std::vector< bool > >& paths ) const
    {
      for( size_t i = 0; i < data_.size(); i++ )
      {
        const std::vector< bool >& path = paths[ i ];
        if( TreeTrainerType::is_blacklisted( blacklist, path ) )
        {
          continue;
        }

        size_t idx = TreeTrainerType::to_int( path );
        for( size_t j = 0; j < random_tests.size(); j++ )
        {
          size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
          bool result = random_tests[ j ]( data_[ i ].input() );
          if( result ) 
          {
            candidate_idx++;
          }

          candidate_statistics[ candidate_idx ]->operator+=( data_[ i ].output() );
        }
      }
    }

    void update_paths( std::vector< std::vector< bool > >& paths,
        const std::vector< std::vector< bool > >& blacklist,
        const TreeType& tree ) const
    {
      for( size_t i = 0; i < data_.size(); i++ )
      {
        std::vector< bool >& path = paths[ i ];
        if( !TreeTrainerType::is_blacklisted( blacklist, path ) )
        {
          path.push_back( tree.get_node( path )->test( data_[ i ].input() ) );
        }
      }
    }

  private:
    size_t                                              num_classes_;
    std::vector< DataPoint< InputType, OutputType > >   data_;
};

#endif
