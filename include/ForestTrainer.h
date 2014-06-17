#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "TrainingContext.h"
#include "Forest.h"
#include "TrainingParameters.h"
#include "TreeTrainer.h"

template< typename D, typename F, typename S >
class ForestTrainer 
{
  private:
    const ITrainingContext< F, S >& context;

  public:
    ForestTrainer( const ITrainingContext< F, S >& c ) :
      context( c )
    {}

    virtual ~ForestTrainer() 
    {}

    Forest< D, F, S > trainForest( const TrainingParameters& params,
        DataRange< D >& range )
    {
      Forest< D, F, S > f( context );
      TreeTrainer< D, F, S > trainer( context );
      for( size_t i=0; i < params.trees; i++ )
      {
        f.add( trainer.trainTree( params, range ) );
      }

      return f;
    }
};

#endif
