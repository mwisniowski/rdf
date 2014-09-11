#ifndef RDF_FOREST_TRAINER_H
#define RDF_FOREST_TRAINER_H

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

    static void train( Forest< F, I, S >& forest, 
        const TrainingContextBase< I, O, S >& context, 
        const TestSamplerBase< F, I >& sampler,
        const std::vector< DataPoint< I, O > >& data )
    {
      for( size_t i = 0; i < context.params().trees; i++ )
      {
        // std::cout << "Training tree " << i + 1 << "/" << context.params().trees << std::endl;
        Tree< F, I, S > tree;
        TreeTrainer< I, O, F, S >::train( tree, context, sampler, data );
        forest.add( tree );
        // std::cout << tree << std::endl;
      }
    }
};

#endif
