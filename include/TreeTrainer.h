#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "Interfaces.h"
#include "Tree.h"
#include "ThresholdSampler.h"

using namespace std;

/**
 * @brief Simple helper class to perform a binary test
 */
template< typename D, typename F, typename S >
struct Test
{
  size_t                              feature_idx;
  float                               threshold;
  const ITrainingContext< D, F, S >&  context;


  Test( const ITrainingContext< D, F, S >& context, float threshold, size_t feature_idx ) :
    context( context ),
    threshold( threshold ),
    feature_idx( feature_idx )
  {}

  bool operator()( size_t data_idx ) const
  {
    return context.lookup( data_idx, feature_idx ) < threshold;
  }
};

template< typename D, typename F, typename S >
class TreeTrainer 
{
  public:
    const ITrainingContext< D, F, S >& context;

  public:
    TreeTrainer( const ITrainingContext< D, F, S >& c ) :
      context( c ) 
    {}

    TreeTrainer( const TreeTrainer& other ) :
      context( other.context )
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
    void train( Tree< D, F, S >& tree ) const
    {
      deque< size_t > frontier;
      frontier.push_back( tree.create_leaf( context.data_idxs ) );

      vector< size_t > feature_idxs;
      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params.max_decision_levels; depth++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          context.get_random_features( feature_idxs );
          size_t node_idx = frontier.front();

          float threshold;
          float gain;
          size_t feature_idx;
          vector< size_t > left_data_idxs, right_data_idxs;

          compute_threshold( threshold, gain, feature_idx, left_data_idxs, right_data_idxs, 
              tree.nodes[ node_idx ].data_idxs, tree.nodes[ node_idx ].statistics, feature_idxs );

          // If information gain high enough convert leaf to split node
          // and add children to frontier queue
          if( !context.should_terminate( gain ) )
          {
            size_t child_offset = tree.convert_to_split( node_idx, threshold, feature_idx, left_data_idxs, right_data_idxs );

            frontier.push_back( node_idx + child_offset );
            frontier.push_back( node_idx + child_offset + 1 );
          } 

          frontier.pop_front();
        }
      }
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
    void compute_threshold( float& best_threshold,
        float& best_gain,
        size_t& best_feature_idx,
        vector< size_t >& best_left_data_idxs,
        vector< size_t >& best_right_data_idxs,
        vector< size_t >& parent_data_idxs,
        const S& parent_statistics,
        const vector< size_t>& feature_idxs ) const
    {
      best_gain = -FLT_MAX;

      for( size_t i = 0; i < feature_idxs.size(); i++ )
      {
        // randomly sample thresholds
        vector< float > candidate_thresholds;
        ThresholdSampler< D, F, S > sampler( context, feature_idxs[ i ], parent_data_idxs );
        sampler.uniform( candidate_thresholds, context.params.no_candate_thresholds );

        Test< D, F, S > test( context, best_threshold, feature_idxs[ i ] );
        for( size_t i = 0; i < context.params.no_candate_thresholds; i++ )
        {
          test.threshold = candidate_thresholds[ i ];

          // partition data for current threshold and evaluate gain
          const vector< size_t >::iterator pivot = 
            std::partition( parent_data_idxs.begin(), parent_data_idxs.end(), test );
          vector< size_t > left_idxs( parent_data_idxs.begin(), pivot ),
            right_idxs( pivot, parent_data_idxs.end() );

          S left_statistics = context.get_statistics();
          S right_statistics = context.get_statistics();
          left_statistics += left_idxs;
          right_statistics += right_idxs;

          float gain = context.compute_information_gain( parent_statistics, left_statistics, right_statistics );
          if( gain > best_gain )
          {
            best_gain = gain;
            best_threshold = test.threshold;
            best_feature_idx = test.feature_idx;
          }
        }
      }

      vector< size_t >::iterator pivot = 
        std::partition( parent_data_idxs.begin(), parent_data_idxs.end(), Test< D, F, S >( context, best_threshold, best_feature_idx ) );
      best_left_data_idxs = vector< size_t >( parent_data_idxs.begin(), pivot );
      best_right_data_idxs = vector< size_t >( pivot, parent_data_idxs.end() );
    }
};

#endif
