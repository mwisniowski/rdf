#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "TrainingContext.h"
#include "IDataPointCollection.h"
#include "TrainingParameters.h"
#include "Tree.h"

using namespace std;
struct Test
{
  const Feature feature;
  float threshold;


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
    TrainingContext context;


  public:
    TreeTrainer( TrainingContext& context ) :
      context( context )
    {}

    TreeTrainer( const TreeTrainer& other ) :
      context( other.context )
    {}

    virtual ~TreeTrainer() 
    {}

    Tree trainTree( const TrainingParameters& params, 
        IDataPointCollection& data ) const
    {
      deque< size_t > frontier;
      IDataPointRange range( data.begin(), data.end() );
      Node n = createLeaf( range );
      Tree tree( n );
      frontier.push_back( 0 );

      for( size_t d = 0; d < params.maxDecisionLevels; d++ )
      {
        size_t current_size = frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          // IDataPointCollection left, right;
          IDataPointRange left_range, right_range;
          Feature feature = context.getRandomFeature();
          size_t node_idx = frontier.front();
          Node& node = tree.nodes.at( node_idx );
          
          float threshold;
          float gain;
          computeThreshold( threshold, gain, left_range,
              right_range, node.data, node.statistics, feature );
          // cout << "Gain: " << gain << endl;
          if( !context.shouldTerminate( gain ) )
          {
            tree.convertToSplit( node_idx, threshold, feature );

            Node left_n = createLeaf( left_range );
            Node right_n = createLeaf( right_range );

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
    Node createLeaf( IDataPointRange& range ) const
    {
      StatisticsAggregator s = context.getStatisticsAggregator();
      s.aggregate( range );
      return Node( s, range );
    }

    void computeThreshold( float& best_threshold,
        float& best_gain,
        IDataPointRange& best_left,
        IDataPointRange& best_right,
        const IDataPointRange& parent,
        StatisticsAggregator& statistics,
        const Feature& feature ) const
    {
      best_threshold = -FLT_MAX;
      best_gain = -FLT_MAX;

      IDataPointRange left( parent ), right( parent );
      Test test( feature, best_threshold );
      IDataPointCollection::const_iterator it = parent.start;
      for( ; it != parent.end; ++it )
      {
        test.threshold = feature( *it );
        left.end = std::partition( parent.start, parent.end, test );
        right.start = left.end;

        StatisticsAggregator left_s = context.getStatisticsAggregator();
        StatisticsAggregator right_s = context.getStatisticsAggregator();

        left_s.aggregate( left );
        right_s.aggregate( right );

        float gain = context.computeInformationGain( statistics, left_s, right_s );
        if( gain > best_gain )
        {
          best_gain = gain;
          best_threshold = test.threshold;
        }
      }

      test.threshold = best_threshold;
      best_left.start = parent.start;
      best_left.end = std::partition( parent.start, parent.end, test );
      best_right.start = best_left.end;
      best_right.end = parent.end;
    }

    // void partition( IDataPointCollection& left, 
    //     IDataPointCollection& right,
    //     const Feature& feature,
    //     float threshold,
    //     const IDataPointCollection& data ) const
    // {
    //   left.clear();
    //   right.clear();
    //
    //   IDataPointCollection::const_iterator it = data.begin(),
    //     end = data.end();
    //   for ( ; it != end; ++it )
    //   {
    //     if( feature( *it ) < threshold )
    //     {
    //       left.push_back( *it );
    //     } 
    //     else 
    //     {
    //       right.push_back( *it );
    //     }
    //   }
    // }
};

#endif
