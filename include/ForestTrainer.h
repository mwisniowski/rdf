#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "TrainingContext.h"
#include "Forest.h"
#include "TrainingParameters.h"
#include "TreeTrainer.h"

class ForestTrainer 
{
  private:
    const TrainingContext _context;

  public:
    ForestTrainer( const TrainingContext& context ) :
      _context( context )
    {}

    virtual ~ForestTrainer() 
    {}

    Forest trainForest( const TrainingParameters& params,
        DataCollection& data )
    {
      Forest f( _context.numClasses );
      TreeTrainer trainer( _context );
      for( size_t i=0; i < params.trees; i++ )
      {
        f.add( trainer.trainTree( params, data ) );
      }

      return f;
    }
};

#endif
