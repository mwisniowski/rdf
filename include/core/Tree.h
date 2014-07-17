#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include <vector>
#include "Interfaces.h"

template< typename D, typename F, typename S >
class Tree 
{
  private:
    class Leaf 
    {
      public:
        Leaf( const S& statistics, const std::vector< size_t > data_idxs ) :
          statistics_( statistics ),
          data_idxs_( data_idxs ),
          split_idx_( 0 )
        {}
    
        Leaf( const Leaf& other ) :
          statistics_( other.statistics_ ),
          data_idxs_( other.data_idxs_ ),
          split_idx_( other.split_idx_)
        {}
    
        virtual ~Leaf() 
        {}
    
        Leaf& operator=( const Leaf& other )
        {
          if( this != &other )
          {
            statistics_ = other.statistics_;
            data_idxs_ = other.data_idxs_;
            split_idx_ = other.split_idx_;
          }
          return *this;
        }

        const S& statistics() const
        {
          return statistics_;
        }

        std::vector< size_t >& data_idxs()
        {
          return data_idxs_;
        }

        size_t split_idx() const
        {
          return split_idx_;
        }

        void set_split_idx( size_t split_idx )
        {
          split_idx_ = split_idx;
        }
    
      private:
        S                      statistics_;
        std::vector< size_t >  data_idxs_;
        size_t                 split_idx_;
    };

    class Split 
    {
      public:
        Split( const F& feature, float threshold, size_t left_idx, size_t right_idx ) :
          feature_( feature ),
          threshold_( threshold ),
          left_leaf_( true ),
          left_idx_( left_idx ),
          right_leaf_( true ),
          right_idx_( right_idx )
        {}

        Split( const Split& other ) :
          feature_( other.feature_ ),
          threshold_( other.threshold_ ),
          left_leaf_( other.left_leaf_ ),
          left_idx_( other.left_idx_ ),
          right_leaf_( other.right_leaf_ ),
          right_idx_( other.right_idx_ )
        {}

        virtual ~Split() 
        {}

        Split& operator=( const Split& other )
        {
          if( this != &other )
          {
            feature_ = other.feature_;
            threshold_ = other.threshold_;
            left_leaf_ = other.left_leaf_;
            left_idx_ = other.left_idx_;
            right_leaf_ = other.right_leaf_;
            right_idx_ = other.right_idx_;
          }
          return *this;
        }

        friend std::ostream& operator<<( std::ostream& os, const Split& n )
        {
          os << n.statistics << " , " << n.child_offset;
          return os;
        }

        const F& feature() const
        {
          return feature_;
        }

        void set_feature_idx( const F& feature )
        {
          feature_ = feature;
        }

        float threshold() const
        {
          return threshold_;
        }

        void set_threshold( float threshold )
        {
          threshold_ = threshold;
        }

        bool left_leaf() const
        {
          return left_leaf_;
        }

        void set_left_leaf( bool leaf_leaf )
        {
          left_leaf_ = leaf_leaf;
        }

        bool right_leaf() const
        {
          return right_leaf_;
        }

        void set_right_leaf( bool right_leaf )
        {
          right_leaf_ = right_leaf;
        }

        size_t left_idx() const
        {
          return left_idx_;
        }

        void set_left_idx( size_t left_idx )
        {
          left_idx_ = left_idx;
        }

        size_t right_idx() const
        {
          return right_idx_;
        }

        void set_right_idx( size_t right_idx )
        {
          right_idx_ = right_idx;
        }

      private:
        F       feature_;
        float   threshold_;
        bool    left_leaf_;
        size_t  left_idx_;
        bool    right_leaf_;
        size_t  right_idx_;
    };


  public:
    Tree()
    {}

    Tree( const Tree& other ) :
      splits_( other.splits_ ),
      leaves_( other.leaves_ )
    {}

    virtual ~Tree() 
    {}

    size_t create_leaf( const TrainingContextBase< D, F, S >& context, const std::vector< size_t >& data_idxs )
    {
      S s = context.get_statistics( data_idxs );
      leaves_.push_back( Leaf( s, data_idxs ) );
      return leaves_.size() - 1;
    }

    void convert_to_split( size_t& left_idx, size_t& right_idx, 
        const TrainingContextBase< D, F, S >& context, size_t leaf_idx, float threshold, size_t feature_idx, 
        const std::vector< size_t >& left_data_idxs, const std::vector< size_t >& right_data_idxs )
    {
      left_idx = create_leaf( context, left_data_idxs );
      right_idx = create_leaf( context, right_data_idxs );

      splits_.push_back( Split( context.feature( feature_idx ), threshold, left_idx, right_idx ) );
      leaves_[ leaf_idx ].set_split_idx( splits_.size() - 1 );
    }

    void prune()
    {
      std::vector< Leaf > pruned_leaves;
      typename std::vector< Split >::iterator it = splits_.begin(),
        end = splits_.end();
      for( ; it != end; ++it )
      {
        if( it->left_leaf() )
        {
          if( leaves_[ it->left_idx() ].split_idx() > 0 )
          {
            it->set_left_leaf( false );
            it->set_left_idx( leaves_[ it->left_idx() ].split_idx() );
          }
          else
          {
            pruned_leaves.push_back( leaves_[ it->left_idx() ] );
            it->set_left_idx( pruned_leaves.size() - 1 );
          }
        }
        if( it->right_leaf() )
        {
          if( leaves_[ it->right_idx() ].split_idx() > 0 )
          {
            it->set_right_leaf( false );
            it->set_right_idx( leaves_[ it->right_idx() ].split_idx() );
          }
          else
          {
            pruned_leaves.push_back( leaves_[ it->right_idx() ] );
            it->set_right_idx( pruned_leaves.size() - 1 );
          }
        }
      }
      leaves_ = pruned_leaves;
    }

    const S& classify( const D& point ) const
    {
      typename std::vector< Split >::const_iterator it = splits_.begin();
      if( it == splits_.end() )
      {
        return leaves_.front().statistics();
      }
      while( true )
      {
        if( it->feature()( point ) < it->threshold() )
        {
          if( it->left_leaf() )
          {
            return leaves_[ it->left_idx() ].statistics();
          } 
          else 
          {
            it = splits_.begin() + it->left_idx();
          }
        } else 
        {
          if( it->right_leaf() )
          {
            return leaves_[ it->right_idx() ].statistics();
          } 
          else
          {
            it = splits_.begin() + it->right_idx();
          }
        }
      }
    }

    std::vector< size_t >& data_idxs( size_t leaf_idx )
    {
      return leaves_[ leaf_idx ].data_idxs();
    }

    const S& statistics( size_t leaf_idx ) const
    {
      return leaves_[ leaf_idx ].statistics();
    }

  private:
    std::vector< Split > splits_;
    std::vector< Leaf > leaves_;

    // std::ostream& preorder( std::ostream& os, size_t node_idx, int level ) const
    // {
    //   if( level )
    //   {
    //     os << std::setw( 4 * level ) << ' ';
    //   }
    //   os << nodes_[ node_idx ] << std::endl;
    //
    //   int offset = nodes_[ node_idx ].child_offset;
    //   if( offset > 0 ) {
    //     preorder( os, node_idx + offset, level + 1 );
    //     preorder( os, node_idx + offset + 1, level + 1 );
    //   }
    //
    //   return os;
    // }
};

#endif
