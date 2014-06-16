#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "TrainingContext.h"
#include "Forest.h"
#include "TrainingParameters.h"
#include "TreeTrainer.h"

class ForestTrainer 
{
  private:
    const TrainingContext context;

  public:
    ForestTrainer( const TrainingContext& context ) :
      context( context )
    {}

    virtual ~ForestTrainer() 
    {}

    Forest trainForest( const TrainingParameters& params,
        DataCollection& data )
    {
      Forest f;
      TreeTrainer trainer( context );
      for( size_t i=0; i < params.trees; i++ )
      {
        f.add( trainer.trainTree( params, data ) );
      }

      return f;
    }
};

#endif
