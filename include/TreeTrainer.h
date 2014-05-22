#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "TrainingContext.h"
#include "IDataPointCollection.h"
#include "TrainingParameters.h"
#include "Tree.h"

using namespace std;
class TreeTrainer 
{
  public:
    TreeTrainer( TrainingContext& context ) :
      context( context )
    {}

    virtual ~TreeTrainer() 
    {}

    Tree trainTree( const TrainingParameters& params, 
        IDataPointCollection& data )
    {
      Node n = createLeaf( data );
      Tree tree( n );
      frontier.push_back( 0 );

      for( size_t d = 0; d < params.maxDecisionLevels; d++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          IDataPointCollection left, right;
          Feature feature = context.getRandomFeature();
          size_t node_idx = frontier.front();
          Node& node = tree.nodes.at( node_idx );
          
          float threshold;
          float gain;
          computeThreshold( threshold, gain, left, right,
              node.data, node.statistics, feature );
          if( !context.shouldTerminate( gain ) )
          {
            tree.convertToSplit( node_idx, threshold, feature );

            Node left_n = createLeaf( left );
            Node right_n = createLeaf( right );

            tree.addLeft( node_idx, left_n );
            tree.addRight( node_idx, right_n );

            frontier.push_back( tree.nodes[ node_idx ].left );
            frontier.push_back( tree.nodes[ node_idx ].right );
          } 

          frontier.pop_front();
        }

        // cout << tree << endl;

      }
      return tree;
    }

  private:
    Node createLeaf( IDataPointCollection& data )
    {
      StatisticsAggregator s = context.getStatisticsAggregator();
      s.aggregate( data );
      return Node( s, data );
    }

    void computeThreshold( float& best_threshold,
        float& best_gain,
        IDataPointCollection& best_left, 
        IDataPointCollection& best_right, 
        IDataPointCollection& data,
        StatisticsAggregator& statistics,
        const Feature& feature )
    {
      best_threshold = -FLT_MAX;
      best_gain = -FLT_MAX;

      IDataPointCollection::iterator it = data.begin(),
        end = data.end();
      for( ; it != end; ++it )
      {
        float threshold = feature( *it );
        IDataPointCollection left, right;
        partition( left, right, feature, threshold, data );

        StatisticsAggregator left_s = context.getStatisticsAggregator();
        StatisticsAggregator right_s = context.getStatisticsAggregator();

        left_s.aggregate( left );
        right_s.aggregate( right );

        float gain = context.computeInformationGain( statistics, left_s, right_s );
        if( gain > best_gain )
        {
          best_gain = gain;
          best_threshold = threshold;
          best_left = left;
          best_right = right;
        }
      }

    }

    void partition( IDataPointCollection& left, 
        IDataPointCollection& right,
        const Feature& feature,
        float threshold,
        const IDataPointCollection& data ) const
    {
      left.clear();
      right.clear();

      IDataPointCollection::const_iterator it = data.begin(),
        end = data.end();
      for ( ; it != end; ++it )
      {
        if( feature( *it ) < threshold )
        {
          left.push_back( *it );
        } 
        else 
        {
          right.push_back( *it );
        }
      }
    }

    deque< size_t > frontier;
    TrainingContext context;
};

#endif
