#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "easylogging++.h"
#include "Forest.h"
#include "TreeTrainer.h"

template< typename D, typename F, typename S >
class ForestTrainer 
{
  public:
    ForestTrainer()
    {}

    virtual ~ForestTrainer() 
    {}

    static void train( Forest< D, F, S >& forest, const TrainingContextBase< D, F, S >& context )
    {
      for( size_t i = 0; i < context.params().trees; i++ )
      {
        LINFO << "Training tree " << i + 1 << "/" << context.params().trees;
        Tree< D, F, S > tree;
        TreeTrainer< D, F, S >::train( tree, context );
        forest.add( tree );
      }
    }
};

#endif
