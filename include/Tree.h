#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include "DataRange.h"
#include "Feature2f.h"
#include "Histogram.h"

template< typename I, typename O, size_t d >
struct Node 
{
  Feature2f     feature;
  Histogram              histogram;
  DataRange< I, O, d >   data;
  float                  threshold;
  int                    childOffset;

  Node( const Histogram& histogram,
      const DataRange< I, O, d >& range ) :
    histogram( histogram ),
    data( range ),
    childOffset( -1 )
  {}

  Node( const Node& other ) :
    feature( other.feature ),
    histogram( other.histogram ),
    threshold( other.threshold ),
    childOffset( other.childOffset ),
    data( other.data )
  {}

  virtual ~Node() 
  {}

  friend ostream& operator<<( ostream& os, const Node& n )
  {
    os << n.histogram << " , " << n.childOffset;
    return os;
  }
};

template< typename I, typename O, size_t d >
class Tree 
{
  public:
    vector< Node< I, O, d > > nodes;


  public:
    Tree( const Node< I, O, d >& root ) 
    {
      nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    const Histogram& classify( const DataPoint< I, O, d >& point ) const
    {
      typename vector< Node< I, O, d > >::const_iterator it = nodes.begin();
      while( it->childOffset > 0 )
      {
        if( it->feature( point ) < it->threshold )
        {
          it += it->childOffset;
        } else {
          it += it->childOffset + 1;
        }
      }
      return it->histogram;
    }

    void convertToSplit( size_t node_idx, float threshold, const Feature< I, O, d >& feature, 
        const Node< I, O, d >& left, const Node< I, O, d >& right )
    {
      Node< I, O, d >& node = nodes.at( node_idx );
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
