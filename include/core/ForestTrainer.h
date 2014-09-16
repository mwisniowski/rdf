#ifndef RDF_FOREST_TRAINER_H
#define RDF_FOREST_TRAINER_H

#include "Forest.h"
#include "TreeTrainer.h"

template< typename C, typename F, typename S, typename TrainerType, typename TreeType >
class ForestTrainer 
{
  public:
    ForestTrainer()
    {}

    virtual ~ForestTrainer() 
    {}

    static void train( F& forest, 
        const C& context, 
        const S& sampler )
    {
      for( size_t i = 0; i < context.params().trees; i++ )
      {
        // std::cout << "Training tree " << i + 1 << "/" << context.params().trees << std::endl;
        TreeType tree;
        TrainerType::train( tree, context, sampler );
        forest.add( tree );
        // std::cout << tree << std::endl;
      }
    }
};

#endif
