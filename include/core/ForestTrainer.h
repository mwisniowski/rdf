#ifndef FOREST_TRAINER_H
#define FOREST_TRAINER_H

#include "Forest.h"
#include "TreeTrainer.h"

template< typename I, typename O, typename F, typename S >
class ForestTrainer 
{
  public:
    ForestTrainer()
    {}

    virtual ~ForestTrainer() 
    {}

    static void train( Forest< I, O, F, S >& forest, const TrainingContextBase< I, O, F, S >& context )
    {
      for( size_t i = 0; i < context.params().trees; i++ )
      {
        // std::cout << "Training tree " << i + 1 << "/" << context.params().trees << std::endl;
        Tree< I, O, F, S > tree;
        TreeTrainer< I, O, F, S >::train( tree, context );
        forest.add( tree );
      }
    }
};

#endif
