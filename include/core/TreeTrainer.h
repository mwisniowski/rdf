#ifndef RDF_TREE_TRAINER_H
#define RDF_TREE_TRAINER_H

#include <deque>
#include <cvt/math/Math.h>

#include "core/Interfaces.h"
#include "core/Tree.h"
#include "core/Test.h"
#include "core/Path.h"

template< typename C, typename S, typename TestType, typename TreeType >
class TreeTrainer 
{
  typedef typename TreeType::Node NodeType;

  public:
    /**
     * @brief While training the tree the elements of the DataCollection will be reordered
     * as part of in-place partitioning.
     * Creates tree in breadth-first order.
     *
     * @param params
     * @param data
     *
     * @return 
     */
    static void train( TreeType& tree, 
        const C& context,
        const TestSamplerBase< TestType >& sampler )
    {
      S root_s = context.get_root_statistics();
      tree.make_root( root_s );
      std::vector< Path > paths( root_s.n(), Path( 0, 0 ) );
      std::vector< Path > blacklist;

      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params().max_depth; depth++ )
      { 
        loadbar( depth, context.params().max_depth );

        // sample tests
        std::vector< TestType > random_tests;
        sampler.sample( random_tests, context.params().tests );

        size_t num_nodes_d = 1UL << depth;

        // generate candidate statistics
        std::vector< S* > candidate_statistics;
        init_candidate_statistics( candidate_statistics, depth, random_tests.size(), blacklist, context );
        context.fill_statistics( candidate_statistics, random_tests, blacklist, paths );

        // compute information gain
        std::vector< S* > selected_statistics( 2 * num_nodes_d );
        std::vector< size_t > selected_test_idxs( num_nodes_d );
        std::vector< float > selected_gains( num_nodes_d, -FLT_MAX );

        compute_information_gains( selected_statistics, selected_test_idxs, selected_gains,
           depth, tree, random_tests, blacklist, context, candidate_statistics );

        // write to tree
        bool growing = false;
        for( size_t i = 0; i < num_nodes_d; i++ )
        {
          Path path( i, depth );
          NodeType* n = tree.get_node( path );
          if( path.is_blacklisted( blacklist ) )
          {
            continue;
          }

          if( !context.should_terminate( selected_gains[ i ] ) )
          {
            tree.convert_to_split( n, random_tests[ selected_test_idxs[ i ] ],
                *selected_statistics[ 2 * i ], *selected_statistics[ 2 * i + 1 ] );
            growing = true;
          }
          else
          {
            blacklist.push_back( path );
          }
        }

        for( size_t i = 0; i < candidate_statistics.size(); i++ )
        {
          delete candidate_statistics[ i ];
        }

        if( !growing )
        {
          return;
        }

        // update paths data has taken
        context.update_paths( paths, blacklist, tree );

        // std::cout << tree << std::endl;
      }
      loadbar( 1, 1 );
      std::cout << std::endl;
    }

  private:
    static void init_candidate_statistics( std::vector< S* >& candidate_statistics,
        size_t depth, 
        size_t num_tests,
        const std::vector< Path >& blacklist,
        const C& context )
    {
      size_t num_nodes_d = 1UL << depth;
      candidate_statistics.clear();
      candidate_statistics.resize( num_nodes_d * num_tests * 2, NULL );

      for( size_t i = 0; i < num_nodes_d; i++ )
      {
        Path path( i, depth );
        if( path.is_blacklisted( blacklist ) )
        {
          continue;
        }

        typename std::vector< S* >::iterator it = candidate_statistics.begin() + i * 2 * num_tests,
                 end = it + 2 * num_tests;
        for( ; it != end; it++ )
        {
          *it = new S( context.get_statistics() );
        }
      }
    }

    static void compute_information_gains( std::vector< S* >& selected_statistics,
        std::vector< size_t >& selected_test_idxs,
        std::vector< float >& selected_gains,
        size_t depth,
        const TreeType& tree,
        const std::vector< TestType >& random_tests,
        const std::vector< Path >& blacklist,
        const C& context,
        std::vector< S* >& candidate_statistics )
    {
      size_t num_nodes_d = 1UL << depth;
      for( size_t i = 0; i < num_nodes_d; i++ )
      {
        Path path( i, depth );
        NodeType* n = tree.get_node( path );
        if( path.is_blacklisted( blacklist ) )
        {
          continue;
        }

        typename std::vector< S* >::iterator it = candidate_statistics.begin() + 
          i * 2 * random_tests.size();
        for( size_t j = 0; j < random_tests.size(); j++, it += 2 )
        {
          S* left = *it;
          S* right = *( it + 1 );
          float gain = context.compute_information_gain( n->statistics, *left, *right );
          if( gain > selected_gains[ i ] )
          {
            selected_gains[ i ] = gain;
            selected_statistics[ 2 * i ] = left;
            selected_statistics[ 2 * i + 1 ] = right;
            selected_test_idxs[ i ] = j;
          }
        }
      }
    }

    static inline void loadbar(unsigned int x, unsigned int n, unsigned int w = 50)
    {
      if ( (x != n) && (x % (n/100+1) != 0) ) return;

      float ratio  =  x/(float)n;
      int   c      =  ratio * w;

      std::cout << std::setw(3) << (int)(ratio*100) << "% [";
      for (int x=0; x<c; x++) std::cout << "=";
      for (int x=c; x<w; x++) std::cout << " ";
      std::cout << "]\r" << std::flush;
    }
};

#endif
