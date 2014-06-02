#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include <deque>

#include "TrainingContext.h"
#include "DataCollection.h"
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

    Tree trainTree( const TrainingParameters& params, 
        DataCollection& data ) const
    {
      deque< size_t > frontier;
      DataRange range( data.begin(), data.end() );
      Node n = createLeaf( range );
      Tree tree( n );
      frontier.push_back( 0 );

      vector< Feature > features;
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

    void computeThreshold( float& best_threshold,
        float& best_gain,
        Feature& best_feature,
        DataRange& best_left,
        DataRange& best_right,
        const DataRange& parent,
        Histogram& histogram,
        const vector< Feature >& features ) const
    {
      best_gain = -FLT_MAX;

      DataRange left( parent ), right( parent );

      vector< Feature >::const_iterator fit = features.begin(),
        fend = features.end();
      for( ; fit != fend; ++fit )
      {
        Test test( *fit, best_threshold );
        DataCollection::const_iterator it = parent.start;
        for( ; it != parent.end; ++it )
        {
          test.threshold = test.feature( *it );
          left.end = std::partition( parent.start, parent.end, test );
          right.start = left.end;

          Histogram left_s = context.getHistogram();
          Histogram right_s = context.getHistogram();

          left_s.aggregate( left );
          right_s.aggregate( right );

          float gain = context.computeInformationGain( histogram, left_s, right_s );
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
