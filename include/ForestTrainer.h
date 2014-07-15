#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "easylogging++.h"
#include "Forest.h"
#include "TreeTrainer.h"

template< typename D, typename F, typename S >
class ForestTrainer 
{
  public:
    ForestTrainer( TrainingContextBase< D, F, S >& c ) :
      context_( c )
    {}

    virtual ~ForestTrainer() 
    {}

    Forest< D, F, S > train()
    {
      Forest< D, F, S > f( context_ );
      TreeTrainer< D, F, S > trainer( context_ );
      for( size_t i=0; i < context_.params().trees; i++ )
      {
        LINFO << "Training tree " << i + 1 << "/" << context_.params().trees;
        Tree< D, F, S > tree( context_ );
        trainer.train( tree );
        f.add( tree );
      }

      return f;
    }

  private:
    TrainingContextBase< D, F, S >& context_;
};

#endif
