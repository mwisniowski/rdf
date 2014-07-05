#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "Forest.h"
#include "TreeTrainer.h"

template< typename D, typename F, typename S >
class ForestTrainer 
{
  private:
    const ITrainingContext< D, F, S >& context;

  public:
    ForestTrainer( const ITrainingContext< D, F, S >& c ) :
      context( c )
    {}

    virtual ~ForestTrainer() 
    {}

    Forest< D, F, S > trainForest( DataRange< D >& range )
    {
      Forest< D, F, S > f( context );
      TreeTrainer< D, F, S > trainer( context );
      for( size_t i=0; i < context.params.trees; i++ )
      {
        f.add( trainer.trainTree( context.params, range ) );
      }

      return f;
    }
};

#endif
