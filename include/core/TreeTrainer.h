#ifndef RDF_TREE_TRAINER_H
#define RDF_TREE_TRAINER_H

#include <deque>
#include <cvt/math/Math.h>

#include "core/Interfaces.h"
#include "core/Tree.h"
#include "core/Test.h"

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
      std::vector< std::vector< bool > > paths( root_s.n(), std::vector< bool >( 0 ) );
      std::vector< std::vector< bool > > blacklist;

      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params().max_depth; depth++ )
      { 
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
        context.update_paths( paths, blacklist, tree );

        // std::cout << tree << std::endl;
      }
    }

    static bool is_blacklisted( const std::vector< std::vector< bool > >& blacklist, 
        const std::vector< bool >& path )
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


  private:
    static void init_candidate_statistics( std::vector< S* >& candidate_statistics,
        size_t depth, 
        size_t num_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const C& context )
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

    static void compute_information_gains( std::vector< S* >& selected_statistics,
        std::vector< size_t >& selected_test_idxs,
        std::vector< float >& selected_gains,
        size_t depth,
        const TreeType& tree,
        const std::vector< TestType >& random_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const C& context,
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


};

#endif
