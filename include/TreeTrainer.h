#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "Interfaces.h"
#include "DataRange.h"
#include "Tree.h"
#include "ThresholdSampler.h"

using namespace std;

/**
 * @brief Simple helper class to perform a binary test
 */
template< typename D, typename F, typename S >
struct Test
{
  const F   feature;
  float     threshold;
  const ITrainingContext< D, F, S >& context;


  Test( const F& f, float t, const ITrainingContext< D, F, S >& c ) :
    feature( f ),
    threshold( t ),
    context( c )
  {}

  bool operator()( const D& point ) const
  {
    return context.lookup( feature, point ) < threshold;
  }
};

template< typename D, typename F, typename S >
class TreeTrainer 
{
  private:
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
    Tree< D, F, S > trainTree( const TrainingParameters& params, 
        DataRange< D >& range ) const
    {
      deque< size_t > frontier;
      Node< D, F, S > n = createLeaf( range );
      Tree< D, F, S > tree( n );
      frontier.push_back( 0 );

      vector< F > features;
      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < params.maxDecisionLevels; depth++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          context.getRandomFeatures( features );
          size_t node_idx = frontier.front();
          Node< D, F, S >& node = tree.nodes[ node_idx ];

          float threshold;
          float gain;
          F feature;
          DataRange< D > left_range, right_range;
          computeThreshold( threshold, gain, feature,
              left_range, right_range, node.data, node.statistics, features );

          // If information gain high enough convert leaf to split node
          // and add children to frontier queue
          if( !context.shouldTerminate( gain ) )
          {
            Node< D, F, S > left_n = createLeaf( left_range );
            Node< D, F, S > right_n = createLeaf( right_range );

            tree.convertToSplit( node_idx, threshold, feature, left_n, right_n);

            frontier.push_back( node_idx + tree.nodes[ node_idx ].childOffset );
            frontier.push_back( node_idx + tree.nodes[ node_idx ].childOffset + 1 );
          } 

          frontier.pop_front();
        }
      }
      return tree;
    }

  private:
    Node< D, F, S > createLeaf( DataRange< D >& range ) const
    {
      S s = context.getStatisticsAggregator();
      s += range;
      return Node< D, F, S >( s, range );
    }

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
    void computeThreshold( float& best_threshold,
        float& best_gain,
        F& best_feature,
        DataRange< D >& best_left,
        DataRange< D >& best_right,
        const DataRange< D >& parent,
        const S& parent_s,
        const vector< F >& features ) const
    {
      best_gain = -FLT_MAX;

      typename vector< F >::const_iterator it = features.begin(),
        end = features.end();
      for( ; it != end; ++it )
      {
        // randomly sample thresholds
        vector< float > candidate_thresholds;
        ThresholdSampler< D, F, S > sampler( context, *it, parent );
        sampler.uniform( candidate_thresholds, context.params.noCandateThresholds );

        Test< D, F, S > test( *it, best_threshold, context );
        for( size_t i = 0; i < context.params.noCandateThresholds; i++ )
        {
          test.threshold = candidate_thresholds[ i ];

          // partition data for current threshold and evaluate gain
          typename DataRange< D >::iterator pivot = std::partition( parent.begin(), parent.end(), test );
          DataRange< D > left( parent.begin(), pivot ),
            right( pivot, parent.end() );

          S left_s = context.getStatisticsAggregator();
          S right_s = context.getStatisticsAggregator();
          left_s += left;
          right_s += right;

          float gain = context.computeInformationGain( parent_s, left_s, right_s );
          if( gain > best_gain )
          {
            best_gain = gain;
            best_threshold = test.threshold;
            best_feature = test.feature;
          }
        }
      }

      typename DataRange< D >::iterator pivot = std::partition( parent.begin(), parent.end(), 
          Test< D, F, S >( best_feature, best_threshold, context ) );
      best_left = DataRange< D >( parent.begin(), pivot );
      best_right = DataRange< D >( pivot, parent.end() );
    }
};

#endif
