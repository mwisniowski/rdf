#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include <vector>
#include "Interfaces.h"

template< typename D, typename F, typename S >
class Tree 
{
  private:
    class Node 
    {
      public:
        Node( const S& statistics, const std::vector< size_t >& data_idxs ) :
          statistics_( statistics ),
          data_idxs_( data_idxs ),
          child_offset_( 0 )
        {}

        Node( const Node& other ) :
          feature_idx_( other.feature_idx_ ),
          statistics_( other.statistics_ ),
          threshold_( other.threshold_ ),
          child_offset_( other.child_offset_ ),
          data_idxs_( other.data_idxs_ )
        {}

        virtual ~Node() 
        {}

        Node& operator=( const Node& other )
        {
          if( this != &other )
          {
            feature_idx_ = other.feature_idx_;
            statistics_ = other.statistics_;
            data_idxs_ = other.data_idxs_;
            threshold_ = other.threshold_;
            child_offset_ = other.child_offset_;
          }
          return *this;
        }

        friend std::ostream& operator<<( std::ostream& os, const Node& n )
        {
          os << n.statistics << " , " << n.child_offset;
          return os;
        }

        const S& statistics() const
        {
          return statistics_;
        }

        std::vector< size_t >& data_idxs()
        {
          return data_idxs_;
        }

        size_t feature_idx() const
        {
          return feature_idx_;
        }

        void set_feature_idx( size_t feature_idx )
        {
          feature_idx_ = feature_idx;
        }

        float threshold() const
        {
          return threshold_;
        }

        void set_threshold( float threshold )
        {
          threshold_ = threshold;
        }

        size_t child_offset() const
        {
          return child_offset_;
        }

        void set_child_offset( size_t child_offset )
        {
          child_offset_ = child_offset;
        }

      private:
        S                      statistics_;
        std::vector< size_t >  data_idxs_;
        size_t                 feature_idx_;
        float                  threshold_;
        size_t                 child_offset_;
    };


  public:
    Tree( TrainingContextBase< D, F, S >& context ) :
      context_( context )
    {}

    virtual ~Tree() 
    {}

    const S& classify( const D& point ) const
    {
      typename std::vector< Node >::const_iterator it = nodes_.begin();
      while( it->child_offset() > 0 )
      {
        const F& feature = context_.feature( it->feature_idx() );
        if( feature( point ) < it->threshold() )
        {
          it += it->child_offset();
        } else {
          it += it->child_offset() + 1;
        }
      }
      return it->statistics();
    }

    size_t convert_to_split( size_t node_idx, float threshold, size_t feature_idx, 
        const std::vector< size_t >& left_data_idxs, const std::vector< size_t >& right_data_idxs )
    {
      size_t offset = create_leaf( left_data_idxs ) - node_idx;
      create_leaf( right_data_idxs );

      nodes_[ node_idx ].set_threshold( threshold );
      nodes_[ node_idx ].set_feature_idx( feature_idx );
      nodes_[ node_idx ].set_child_offset( offset );

      return offset;
    }

    size_t create_leaf( const std::vector< size_t >& data_idxs )
    {
      S s = context_.get_statistics();
      s += data_idxs;
      nodes_.push_back( Node( s, data_idxs ) );
      return nodes_.size() - 1;
    }

    friend std::ostream& operator<<( std::ostream& os, const Tree& t )
    {
      return t.preorder( os, 0, 0 );
    }

    std::vector< size_t >& data_idxs( size_t node_idx )
    {
      return nodes_[ node_idx ].data_idxs();
    }

    const S& statistics( size_t node_idx ) const
    {
      return nodes_[ node_idx ].statistics();
    }

  private:
    std::vector< Node >           nodes_;
    TrainingContextBase< D, F, S >&  context_;

    std::ostream& preorder( std::ostream& os, size_t node_idx, int level ) const
    {
      if( level )
      {
        os << std::setw( 4 * level ) << ' ';
      }
      os << nodes_[ node_idx ] << std::endl;

      int offset = nodes_[ node_idx ].child_offset;
      if( offset > 0 ) {
        preorder( os, node_idx + offset, level + 1 );
        preorder( os, node_idx + offset + 1, level + 1 );
      }

      return os;
    }
};

#endif
