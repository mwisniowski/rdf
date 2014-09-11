#ifndef RDF_TREE_TRAINER_H
#define RDF_TREE_TRAINER_H

#include <deque>
#include <cvt/math/Math.h>

#include "core/Interfaces.h"
#include "core/Tree.h"
#include "core/Test.h"

template< typename I, typename O, typename F, typename S >
class TreeTrainer 
{
  typedef typename Tree< F, I, S >::Node NodeType;
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
    static void train( Tree< F, I, S >& tree, 
        const TrainingContextBase< I, O, S >& context,
        const TestSamplerBase< F, I >& sampler,
        const std::vector< DataPoint< I, O > >& data )
    {
      tree.make_root( context.get_statistics( data ) );
      std::vector< std::vector< bool > > paths( data.size(), std::vector< bool >( 0 ) );
      std::vector< std::vector< bool > > blacklist;

      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params().max_depth; depth++ )
      { 
        // sample tests
        std::vector< Test< F, I > > random_tests;
        sampler.sample( random_tests, context.params().tests );

        /**********
         * Start GPU
         **********/

        size_t num_nodes_d = 1UL << depth;

        // generate candidate statistics
        std::vector< S* > candidate_statistics;
        init_candidate_statistics( candidate_statistics, depth, random_tests.size(), blacklist, context );
        fill_statistics( candidate_statistics, random_tests, tree, paths, blacklist, data );

        // compute information gain
        std::vector< S* > selected_statistics( 2 * num_nodes_d );
        std::vector< size_t > selected_test_idxs( num_nodes_d );
        std::vector< float > selected_gains( num_nodes_d, -FLT_MAX );

        compute_information_gains( selected_statistics, selected_test_idxs, selected_gains,
           depth, tree, blacklist, random_tests, context, candidate_statistics );
        
        /**********
         * End GPU
         **********/

        // write to tree
        bool growing = false;
        for( size_t i = 0; i < num_nodes_d; i++ )
        {
          std::vector< bool > path = to_path( i, depth );
          NodeType* n = tree.get_node( path );
          if( is_blacklisted( blacklist, path ) )
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
        for( size_t i = 0; i < data.size(); i++ )
        {
          std::vector< bool >& path = paths[ i ];
          NodeType* n = tree.get_node( path );
          if( !is_blacklisted( blacklist, path ) )
          {
            path.push_back( n->test( data[ i ].input() ) );
          }
        }

        // std::cout << tree << std::endl;
      }
    }

  private:
    static void init_candidate_statistics( std::vector< S* >& candidate_statistics,
        size_t depth, size_t num_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const TrainingContextBase< I, O, S >& context )
    {
      size_t num_nodes_d = 1UL << depth;
      candidate_statistics.clear();
      candidate_statistics.resize( num_nodes_d * num_tests * 2, NULL );

      for( size_t i = 0; i < num_nodes_d; i++ )
      {
        std::vector< bool > path = to_path( i, depth );
        if( is_blacklisted( blacklist, path ) )
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

    static void fill_statistics( std::vector< S* >& candidate_statistics,
        const std::vector< Test< F, I > >& random_tests,
        const Tree< F, I, S >& tree,
        const std::vector< std::vector< bool > >& paths,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< DataPoint< I, O > >& data )
    {
      for( size_t i = 0; i < data.size(); i++ )
      {
        const std::vector< bool >& path = paths[ i ];
        NodeType* n = tree.get_node( path );
        if( is_blacklisted( blacklist, path ) )
        {
          continue;
        }

        size_t idx = to_int( path );
        for( size_t j = 0; j < random_tests.size(); j++ )
        {
          size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
          bool result = random_tests[ j ]( data[ i ].input() );
          if( result ) 
          {
            candidate_idx++;
          }

          candidate_statistics[ candidate_idx ]->operator+=( data[ i ].output() );
        }
      }
    }

    static void compute_information_gains( std::vector< S* >& selected_statistics,
        std::vector< size_t >& selected_test_idxs,
        std::vector< float >& selected_gains,
        size_t depth,
        const Tree< F, I, S >& tree,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< Test< F, I > >& random_tests,
        const TrainingContextBase< I, O, S >& context,
        std::vector< S* >& candidate_statistics )
    {
      size_t num_nodes_d = 1UL << depth;
      for( size_t i = 0; i < num_nodes_d; i++ )
      {
        std::vector< bool > path = to_path( i, depth );
        NodeType* n = tree.get_node( path );
        if( is_blacklisted( blacklist, path ) )
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

    static bool is_blacklisted( const std::vector< std::vector< bool > >& blacklist, const std::vector< bool >& path )
    {
      for( size_t i = 0; i < blacklist.size(); i++ )
      {
        const std::vector< bool >& p = blacklist[ i ];
        int j = 0;
        for( ; j < p.size() && path[ j ] == p[ j ]; j++ )
        {}
        if( j == p.size() )
        {
          return true;
        }
      }
      return false;
    }

    static std::vector< bool > to_path( size_t x, size_t depth )
    {
      std::vector< bool > path( depth );
      if( depth )
      {
        size_t mask = 1UL << ( depth - 1 );
        for( size_t i = 0; i < depth; i++ )
        {
          path[ i ] = x & mask;
          mask >>= 1;
        }
      }
      return path;
    }

    static size_t to_int( const std::vector< bool >& path )
    {
      size_t idx = 0;
      for( size_t j = 0; j < path.size(); j++ )
      {
        idx += path[ j ] << path.size() - 1 - j;
      }
      return idx;
    }
};

#endif
