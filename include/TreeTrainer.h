#ifndef TREE_TRAINER_H
#define TREE_TRAINER_H

#include "ITrainingContext.h"
#include "IDataPointCollection.h"
#include "TrainingParameters.h"
#include "Tree.h"

class TreeTrainer 
{
  public:
    TreeTrainer()
    {}

    virtual ~TreeTrainer() 
    {}

    void trainTree( const TrainingParameters& params, 
        const ITrainingContext& context, 
        const IDataPointCollection& data )
    {
      // TODO
      const IFeature& feature = context.getRandomFeature();
      float threshold = findThreshold( feature, data );
      Node n( feature, threshold );
      Tree t( n );
    }

  private:
    float findThreshold( const IFeature& feature, const IDataPointCollection& data )
    {
      //TODO
      return 0.0f;
    }

    void partition( IDataPointCollection& left, 
        IDataPointCollection& right,
        const IFeature& feature,
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
};

#endif
