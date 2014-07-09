#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include "DataRange.h"

template< typename D, typename F, typename S >
struct Node 
{
  F               feature;
  S               statistics;
  DataRange< D >  data;
  float           threshold;
  size_t          childOffset;

  Node( const S& s, const DataRange< D >& range ) :
    statistics( s ),
    data( range ),
    childOffset( 0 )
  {}

  Node( const Node& other ) :
    feature( other.feature ),
    statistics( other.statistics ),
    threshold( other.threshold ),
    childOffset( other.childOffset ),
    data( other.data )
  {}

  virtual ~Node() 
  {}

  Node& operator=( const Node& other )
  {
    if( this != &other )
    {
      feature = other.feature;
      statistics = other.statistics;
      data = other.data;
      threshold = other.threshold;
      childOffset = other.childOffset;
    }
    return *this;
  }

  friend ostream& operator<<( ostream& os, const Node& n )
  {
    os << n.statistics << " , " << n.childOffset;
    return os;
  }
};

template< typename D, typename F, typename S >
class Tree 
{
  public:
    vector< Node< D, F, S > > nodes;


  public:
    Tree( const Node< D, F, S >& root ) 
    {
      nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    const S& classify( const D& point ) const
    {
      typename vector< Node< D, F, S > >::const_iterator it = nodes.begin();
      while( it->childOffset > 0 )
      {
        if( it->feature( point ) < it->threshold )
        {
          it += it->childOffset;
        } else {
          it += it->childOffset + 1;
        }
      }
      return it->statistics;
    }

    void convertToSplit( size_t node_idx, float threshold, const F& feature, 
        const Node< D, F, S >& left, const Node< D, F, S >& right )
    {
      Node< D, F, S >& node = nodes.at( node_idx );
      node.threshold = threshold;
      node.feature = feature;
      node.childOffset = nodes.size() - node_idx;
      nodes.push_back( left );
      nodes.push_back( right );
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

      int offset = nodes[ node_idx ].childOffset;
      if( offset > 0 ) {
        preorder( os, node_idx + offset, level + 1 );
        preorder( os, node_idx + offset + 1, level + 1 );
      }

      return os;
    }
};

#endif
