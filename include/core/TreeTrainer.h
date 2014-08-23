#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>
#include <cvt/math/Math.h>

#include "Interfaces.h"
#include "Tree.h"

template< typename D, typename F, typename S >
class TreeTrainer 
{
  private:
    /**
    * @brief Simple helper class to perform a binary test
    */
    class Test
    {
      public:
        Test( const TrainingContextBase< D, F, S >& context, float threshold, size_t feature_idx ) :
          context_( context ),
          threshold_( threshold ),
          feature_idx_( feature_idx )
        {}

        bool operator()( size_t data_idx ) const
        {
          return context_.lookup( data_idx, feature_idx_ ) < threshold_;
        }

        void set_threshold( float threshold )
        {
          threshold_ = threshold;
        }

      private:
        const TrainingContextBase< D, F, S >&  context_;
        size_t                              feature_idx_;
        float                               threshold_;
    };

    class ThresholdSampler
    { 
      public:
        ThresholdSampler( size_t feature_idx, const std::vector< size_t >& data_idxs ) :
          feature_idx_( feature_idx ),
          data_idxs_( data_idxs )
        {}

        /**
        * @brief Samples thresholds uniformly between minimum and maximum
        *
        * @param thresholds
        * @param size
        */
        void uniform( std::vector< float >& thresholds, size_t size, const TrainingContextBase< D, F, S >& context ) const
        {
          thresholds.clear();
          thresholds.reserve( size );
          float min, max;
          get_min_max( min, max, context );
          for( size_t i = 0; i < size; i++ )
          {
            thresholds.push_back( cvt::Math::rand( min, max ) );
          }
        }

      private:
        const size_t                        feature_idx_;
        const std::vector< size_t >         data_idxs_;

        /**
        * @brief Finds minimum and maximum feature value in DataCollection
        *
        * @param min
        * @param max
        * @param range
        */
        void get_min_max( float& min, float& max, const TrainingContextBase< D, F, S >& context ) const
        {
          min = FLT_MAX;
          max = -min;

          for( size_t i = 0; i < data_idxs_.size(); i++ )
          {
            float response = context.lookup( data_idxs_[ i ], feature_idx_ );
            if( response < min )
            {
              min = response;
            }
            if( response > max )
            {
              max = response;
            }
          }
        }
    };


  public:
    TreeTrainer()
    {}

    TreeTrainer( const TreeTrainer& other )
    {}

    virtual ~TreeTrainer() 
    {}

    /**
     * @brief While training the tree the elements of the DataCollection will be reordered
     * as part of in-place partitioning.
     * Creates tree in breadth-first order.
     *
     * @param params
     * @param data
     *
     * @return 
     */
    static void train( Tree< D, F, S >& tree, const TrainingContextBase< D, F, S >& context )
    {
      std::deque< size_t > frontier;
      frontier.push_back( tree.create_leaf( context, context.get_data_idxs() ) );

      std::vector< size_t > feature_idxs;
      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params().max_decision_levels; depth++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          context.get_random_features( feature_idxs );
          size_t leaf_idx = frontier.front();

          float threshold;
          float gain;
          size_t feature_idx;
          std::vector< size_t > left_data_idxs, right_data_idxs;

          compute_threshold( threshold, gain, feature_idx, left_data_idxs, right_data_idxs, 
              context, tree.data_idxs( leaf_idx ), tree.statistics( leaf_idx ), feature_idxs );

          // If information gain high enough, convert leaf to split node and add children to frontier queue
          if( !context.should_terminate( gain ) )
          {
            size_t left_idx, right_idx;
            tree.convert_to_split( left_idx, right_idx, context, leaf_idx, threshold, feature_idx, left_data_idxs, right_data_idxs );

            frontier.push_back( left_idx );
            frontier.push_back( right_idx );
          } 

          frontier.pop_front();
        }
      }

      tree.prune();
    }

  private:

    /**
     * @brief Selects the best threshold for the given data-partition (parent) according
     * to information gain. Returns the optimal threshold, information gain, feature function
     * and partition
     *
     * @param best_threshold return value
     * @param best_gain return value
     * @param best_feature return value
     * @param best_left return value, left partition of data ( f(x) < t )
     * @param best_right return value, right partition of data ( f(x) >= t )
     * @param parent data partition for which to compute split
     * @param parent_s statistics of data partition
     * @param features feature functions to be evaluated
     */
    static void compute_threshold( float& best_threshold,
        float& best_gain,
        size_t& best_feature_idx,
        std::vector< size_t >& best_left_data_idxs,
        std::vector< size_t >& best_right_data_idxs,
        const TrainingContextBase< D, F, S >& context,
        std::vector< size_t >& parent_data_idxs,
        S& parent_statistics,
        const std::vector< size_t>& feature_idxs )
    {
      best_gain = -FLT_MAX;

      for( size_t i = 0; i < feature_idxs.size(); i++ )
      {
        // randomly sample thresholds
        std::vector< float > candidate_thresholds;
        ThresholdSampler sampler( feature_idxs[ i ], parent_data_idxs );
        sampler.uniform( candidate_thresholds, context.params().no_candate_thresholds, context );

        Test test( context, best_threshold, feature_idxs[ i ] );
        for( size_t j = 0; j < context.params().no_candate_thresholds; j++ )
        {
          test.set_threshold( candidate_thresholds[ j ] );

          // partition data for current threshold and evaluate gain
          const std::vector< size_t >::iterator pivot =
            std::partition( parent_data_idxs.begin(), parent_data_idxs.end(), test );
          std::vector< size_t > left_idxs( parent_data_idxs.begin(), pivot ),
            right_idxs( pivot, parent_data_idxs.end() );

          S left_statistics = context.get_statistics( left_idxs );
          S right_statistics = context.get_statistics( right_idxs );

          float gain = context.compute_information_gain( parent_statistics, left_statistics, right_statistics );
          if( gain > best_gain )
          {
            best_gain = gain;
            best_threshold = candidate_thresholds[ j ];
            best_feature_idx = feature_idxs[ i ];
          }
        }
      }

      std::vector< size_t >::iterator pivot =
        std::partition( parent_data_idxs.begin(), parent_data_idxs.end(), Test( context, best_threshold, best_feature_idx ) );
      best_left_data_idxs = std::vector< size_t >( parent_data_idxs.begin(), pivot );
      best_right_data_idxs = std::vector< size_t >( pivot, parent_data_idxs.end() );
    }
};

#endif
