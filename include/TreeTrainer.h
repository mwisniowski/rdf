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
      _context( context )
    {}

    virtual ~TreeTrainer() 
    {}

    Tree trainTree( const TrainingParameters& params, 
        IDataPointCollection& data )
    {
      Node n = createLeaf( data );
      _frontier.push_back( n );
      Tree t( n );

      for( size_t i = 0; i < params.maxDecisionLevels; i++ )
      {
        size_t current_size = _frontier.size();
        for( size_t i = 0; i < current_size; i++ )
        {
          IDataPointCollection left, right;
          Feature feature = _context.getRandomFeature();
          n = _frontier.front();
          
          if( convertToSplit( n, left, right, feature ) )
          {
            Node left_n = createLeaf( left );
            Node right_n = createLeaf( right );

            _frontier.push_back( left_n );
            _frontier.push_back( right_n );

            t.addLeft( n, left_n );
            t.addRight( n, right_n );
          }

          _frontier.pop_front();
        }
      }

      return t;
    }

  private:
    Node createLeaf( IDataPointCollection& data )
    {
      StatisticsAggregator s = _context.getStatisticsAggregator();
      s.aggregate( data );
      return Node( s, data );
    }

    bool convertToSplit( Node& node,
        IDataPointCollection& left, 
        IDataPointCollection& right, 
        const Feature& feature )
    {
      float best_threshold;
      float best_gain;

      IDataPointCollection best_left, best_right;

      IDataPointCollection::const_iterator it = node._data.begin(),
        end = node._data.end();
      for( ; it != end; ++it )
      {
        float threshold = feature( *node._data.begin() );
        partition( left, right, feature, threshold, node._data );

        StatisticsAggregator left_s = _context.getStatisticsAggregator();
        StatisticsAggregator right_s = _context.getStatisticsAggregator();

        left_s.aggregate( left );
        right_s.aggregate( right );

        float gain = _context.computeInformationGain( node._statistics, left_s, right_s );
        if( gain > best_gain )
        {
          best_gain = gain;
          best_threshold = threshold;
          best_left = left;
          best_right = right;
        }
      }

      if( _context.shouldTerminate( best_gain ) )
      {
        return false;
      }

      node._threshold = best_threshold;
      node._feature = feature;
      node._data = IDataPointCollection();
      node._statistics = StatisticsAggregator();

      return true;
    }

    void partition( IDataPointCollection& left, 
        IDataPointCollection& right,
        const Feature& feature,
        float threshold,
        const IDataPointCollection& data ) const
    {
      left.clear();
      right.clear();

      for (size_t i = 0; i < data.size(); ++i)
      {
        DataPoint2f instance = data[ i ];

        if( feature( instance ) < threshold )
        {
          left.push_back( instance );
        } 
        else 
        {
          right.push_back( instance );
        }
      }
    }

    deque< Node > _frontier;
    TrainingContext _context;
};

#endif
