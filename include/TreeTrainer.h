#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "TrainingContext.h"
#include "DataCollection.h"
#include "TrainingParameters.h"
#include "Tree.h"
#include "ThresholdSampler.h"

using namespace std;

/**
 * @brief Simple helper class to perform a binary test
 */
struct Test
{
  const Feature   feature;
  float           threshold;


  Test( const Feature& f, float t ) :
    feature( f ),
    threshold( t )
  {}

  bool operator()( const DataPoint2f& point ) const
  {
    return feature( point ) < threshold;
  }
};

class TreeTrainer 
{
  private:
    const TrainingContext context;


  public:
    TreeTrainer( const TrainingContext& context ) :
      context( context ) 
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
    Tree trainTree( const TrainingParameters& params, 
        DataCollection& data ) const
    {
      deque< size_t > frontier;
      DataRange range( data.begin(), data.end() );
      Node n = createLeaf( range );
      Tree tree( n );
      frontier.push_back( 0 );

      vector< Feature > features;
      // At every tree level expand all frontier nodes
      for( size_t d = 0; d < params.maxDecisionLevels; d++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          context.getRandomFeatures( features );
          size_t node_idx = frontier.front();
          Node& node = tree.nodes[ node_idx ];

          float threshold;
          float gain;
          Feature feature;
          DataRange left_range, right_range;
          computeThreshold( threshold, gain, feature,
              left_range, right_range, node.data, node.histogram, features );

          // If information gain high enough convert leaf to split node
          // and add children to frontier queue
          if( !context.shouldTerminate( gain ) )
          {
            Node left_n = createLeaf( left_range );
            Node right_n = createLeaf( right_range );

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
    Node createLeaf( DataRange& range ) const
    {
      Histogram s = context.getHistogram();
      s.aggregate( range );
      return Node( s, range );
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
        Feature& best_feature,
        DataRange& best_left,
        DataRange& best_right,
        const DataRange& parent,
        const Histogram& parent_s,
        const vector< Feature >& features ) const
    {
      best_gain = -FLT_MAX;

      DataRange left( parent ), right( parent );

      vector< Feature >::const_iterator it = features.begin(),
        end = features.end();
      for( ; it != end; ++it )
      {
        // randomly sample thresholds
        vector< float > candidate_thresholds;
        ThresholdSampler sampler( *it, parent );
        sampler.uniform( candidate_thresholds, context.params.noCandateThresholds );

        Test test( *it, best_threshold );
        for( size_t i = 0; i < context.params.noCandateThresholds; i++ )
        {
          test.threshold = candidate_thresholds[ i ];

          // partition data for current threshold and evaluate gain
          left.end = std::partition( parent.start, parent.end, test );
          right.start = left.end;

          Histogram left_s = context.getHistogram();
          left_s.aggregate( left );
          Histogram right_s = context.getHistogram();
          right_s.aggregate( right );

          float gain = context.computeInformationGain( parent_s, left_s, right_s );
          if( gain > best_gain )
          {
            best_gain = gain;
            best_threshold = test.threshold;
            best_feature = test.feature;
          }
        }
      }

      best_left.start = parent.start;
      best_left.end = std::partition( parent.start, parent.end, 
          Test( best_feature, best_threshold ) );
      best_right.start = best_left.end;
      best_right.end = parent.end;
    }
};

#endif
