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
  typedef typename Tree< I, O, F, S >::Node NodeType;
  public:
    // TreeTrainer()
    // {}
    //
    // TreeTrainer( const TreeTrainer& other )
    // {}
    //
    // virtual ~TreeTrainer() 
    // {}

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
    static void train( Tree< I, O, F, S >& tree, 
        const TrainingContextBase< I, O, S >& context,
        const TestSamplerBase< I, O, F >& sampler,
        const std::vector< DataPoint< I, O > >& data )
    {
      tree.make_root( context.get_statistics( data ) );
      std::vector< std::vector< bool > > paths( data.size(), std::vector< bool >( 0 ) );
      std::vector< std::vector< bool > > blacklist;

      // At every tree level expand all frontier nodes
      for( size_t depth = 0; depth < context.params().max_decision_levels; depth++ )
      { 
        // sample tests
        std::vector< Test< I, O, F > > random_tests;
        sampler.sample( random_tests, context.params().no_candidate_features );

        /**********
         * Start GPU
         **********/

        size_t num_nodes_d = 1UL << depth;

        // generate candidate statistics
        // std::vector< S > candidate_statistics( num_nodes_d * random_tests.size() * 2, context.get_statistics() );
        std::vector< std::vector< std::pair< S, S > > > candidate_statistics( 
          num_nodes_d, 
          std::vector< std::pair< S, S > >( 
            random_tests.size(), 
            std::pair< S, S >( context.get_statistics(), context.get_statistics() ) 
          ) 
        );
        // aggregate data in respective bin
        fill_statistics( candidate_statistics, random_tests, tree, paths, blacklist, data );

        // compute information gain
        // std::vector< S > selected_statistics( 2 * num_nodes_d, context.get_statistics() );
        std::vector< std::pair< S, S > > selected_statistics( 
          num_nodes_d, 
          std::pair< S, S >( context.get_statistics(), context.get_statistics() ) 
        );
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
                selected_statistics[ i ].first, selected_statistics[ i ].second );
            growing = true;
          }
          else
          {
            blacklist.push_back( path );
          }
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
            path.push_back( n->test( data[ i ] ) );
          }
        }

        // std::cout << tree << std::endl;
      }
    }

  private:
    static void fill_statistics( std::vector< std::vector< std::pair< S, S > > >& candidate_statistics,
        const std::vector< Test< I, O, F > >& random_tests,
        const Tree< I, O, F, S >& tree,
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
          // size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
          std::pair< S, S >& s_pair = candidate_statistics[ idx ][ j ];
          bool result = random_tests[ j ]( data[ i ] );
          if( result ) 
          {
            s_pair.second += data[ i ].output();
            // candidate_idx++;
          }
          else
          {
            s_pair.first += data[ i ].output();
          }

          // candidate_statistics[ candidate_idx ] += data[ i ].output();
        }
      }
    }

    static void compute_information_gains( std::vector< std::pair< S, S > >& selected_statistics,
        std::vector< size_t >& selected_test_idxs,
        std::vector< float >& selected_gains,
        size_t depth,
        const Tree< I, O, F, S >& tree,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< Test< I, O, F > >& random_tests,
        const TrainingContextBase< I, O, S >& context,
        std::vector< std::vector< std::pair< S, S > > >& candidate_statistics )
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

        for( size_t j = 0; j < random_tests.size(); j++ )
        {
          std::pair< S, S >& s_pair = candidate_statistics[ i ][ j ];
          float gain = context.compute_information_gain( n->statistics, s_pair.first, s_pair.second );
          if( gain > selected_gains[ i ] )
          {
            selected_gains[ i ] = gain;
            selected_statistics[ i ] = s_pair;
            selected_test_idxs[ i ] = j;
          }
        }

        // typename std::vector< S >::iterator it = candidate_statistics.begin() + 
        //   i * 2 * random_tests.size();
        // for( size_t j = 0; j < random_tests.size(); j++, it += 2 )
        // {
        //   S& left = *it;
        //   S& right = *( it + 1 );
        //   float gain = context.compute_information_gain( n->statistics, left, right );
        //   if( gain > selected_gains[ i ] )
        //   {
        //     selected_gains[ i ] = gain;
        //     selected_statistics[ 2 * i ] = left;
        //     selected_statistics[ 2 * i + 1 ] = right;
        //     selected_test_idxs[ i ] = j;
        //   }
        // }
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
