#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include <vector>
#include "Interfaces.h"

using namespace std;
template< typename F, typename S >
struct Node 
{
  const S           statistics;
  vector< size_t >  data_idxs;
  size_t            feature_idx;
  float             threshold;
  size_t            child_offset;

  Node( const S& statistics, const vector< size_t >& data_idxs ) :
    statistics( statistics ),
    data_idxs( data_idxs ),
    child_offset( 0 )
  {}

  Node( const Node& other ) :
    feature_idx( other.feature_idx ),
    statistics( other.statistics ),
    threshold( other.threshold ),
    child_offset( other.child_offset ),
    data_idxs( other.data_idxs )
  {}

  virtual ~Node() 
  {}

  Node& operator=( const Node& other )
  {
    if( this != &other )
    {
      feature_idx = other.feature_idx;
      statistics = other.statistics;
      data_idxs = other.data_idxs;
      threshold = other.threshold;
      child_offset = other.child_offset;
    }
    return *this;
  }

  friend ostream& operator<<( ostream& os, const Node& n )
  {
    os << n.statistics << " , " << n.child_offset;
    return os;
  }
};

template< typename D, typename F, typename S >
class Tree 
{
  public:
    vector< Node< F, S > > nodes;
    const ITrainingContext< D, F, S >& context;

  public:
    Tree( const ITrainingContext< D, F, S >& context ) :
      context( context )
    {
    }

    virtual ~Tree() 
    {}

    const S& classify( const D& point ) const
    {
      typename vector< Node< F, S > >::const_iterator it = nodes.begin();
      while( it->child_offset > 0 )
      {
        if( context.feature_pool[ it->feature_idx ]( point ) < it->threshold )
        {
          it += it->child_offset;
        } else {
          it += it->child_offset + 1;
        }
      }
      return it->statistics;
    }

    size_t convert_to_split( size_t node_idx, float threshold, size_t feature_idx, 
        const vector< size_t >& left_data_idxs, const vector< size_t >& right_data_idxs )
    {
      size_t offset = create_leaf( left_data_idxs ) - node_idx;
      create_leaf( right_data_idxs );

      nodes[ node_idx ].threshold = threshold;
      nodes[ node_idx ].feature_idx = feature_idx;
      nodes[ node_idx ].child_offset = offset;

      return offset;
    }

    size_t create_leaf( const vector< size_t >& data_idxs )
    {
      S s = context.get_statistics();
      s += data_idxs;
      nodes.push_back( Node< F, S >( s, data_idxs ) );
      return nodes.size() - 1;
    }

    friend ostream& operator<<( ostream& os, const Tree& t )
    {
      return t.preorder( os, 0, 0 );
    }

  private:
    ostream& preorder( ostream& os, size_t node_idx, int level ) const
    {
      if( level )
      {
        os << std::setw( 4 * level ) << ' ';
      }
      os << nodes[ node_idx ] << endl;

      int offset = nodes[ node_idx ].child_offset;
      if( offset > 0 ) {
        preorder( os, node_idx + offset, level + 1 );
        preorder( os, node_idx + offset + 1, level + 1 );
      }

      return os;
    }
};

#endif
