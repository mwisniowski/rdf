#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include "DataCollection.h"
#include "Feature.h"
#include "Histogram.h"

struct Node 
{
  Feature     feature;
  Histogram   histogram;
  DataRange   data;
  float       threshold;
  int         childOffset;

  Node( const Histogram& histogram,
      const DataRange& range ) :
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

class Tree 
{
  public:
    vector< Node > nodes;


  public:
    Tree( const Node& root ) 
    {
      nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    const Histogram& classify( const DataPoint2f& point ) const
    {
      vector< Node >::const_iterator it = nodes.begin();
      while( it->childOffset > 0 )
      {
        if( it->feature( point ) < it->threshold )
        {
          it += it->childOffset;
        } else {
          it += it->childOffset + 1;
        }
        // it += it->childOffset + ( it->feature( point ) < it->threshold );
      }
      // u_int c = it->statistics.maxClass();
      // return pair< u_int, float >( c, it->statistics.probability( c ) );
      return it->histogram;
    }

    void convertToSplit( size_t node_idx, float threshold, const Feature& feature, 
        const Node& left, const Node& right )
    {
      Node& node = nodes.at( node_idx );
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
