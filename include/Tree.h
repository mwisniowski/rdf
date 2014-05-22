#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include "Feature.h"
#include "StatisticsAggregator.h"

struct Node 
{
  Node()
  {
    init();
  }

  Node( const Feature& feature,
      float threshold ) :
    feature( feature ),
    threshold( threshold )
  {
    init();
  }

  Node( const StatisticsAggregator& statistics,
      const IDataPointCollection& data ) :
    statistics( statistics ),
    data( data ),
    split( false )
  {
    init();
  }

  Node( const Node& other ) :
    feature( other.feature ),
    statistics( other.statistics ),
    threshold( other.threshold ),
    left( other.left ),
    right( other.right ),
    data( other.data ),
    split( other.split )
  {}

  Node& operator=( const Node& other )
  {
    if( this != &other )
    {
      feature = other.feature;
      statistics = other.statistics;
      threshold = other.threshold;
      left = other.left;
      right = other.right;
      data = other.data;
      split = other.split;
    }
    return *this;
  }

  virtual ~Node() 
  {}

  void init()
  {
    left = -1;
    right = -1;
  }

  friend ostream& operator<<( ostream& os, const Node& n )
  {
    os << n.statistics << " - (" << n.left << "," << n.right << ")";
    return os;
  }

  Feature              feature;
  StatisticsAggregator statistics;
  IDataPointCollection data; //TODO Inplace partitioning
  float                threshold;
  int                  left;
  int                  right;
  bool                 split;
};

class Tree 
{
  public:
    Tree( Node& root ) 
    {
      nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    void addLeft( size_t parent, Node& child )
    {
      nodes.push_back( child );
      nodes.at( parent ).left = nodes.size() - 1;
    }

    void addRight( size_t parent, Node& child )
    {
      nodes.push_back( child );
      nodes.at( parent ).right = nodes.size() - 1;
    }

    pair< u_int, float > classify( DataPoint2f& point )
    {
      Node n = nodes[ 0 ];
      // cout << "Classification trace: ";
      while( n.split )
      {
        if( n.feature( point ) < n.threshold )
        {
          // cout << "left ";
          n = nodes[ n.left ];
        } else {
          // cout << "right ";
          n = nodes[ n.right ];
        }
      }
      // cout << endl;

      u_int c = n.statistics.maxClass();
      return pair< u_int, float >( c, n.statistics.probability( c ) );
    }

    void convertToSplit( size_t node_idx, float threshold, Feature& feature )
    {
      Node& node = nodes.at( node_idx );
      node.threshold = threshold;
      node.feature = feature;
      node.data = IDataPointCollection();
      node.statistics = StatisticsAggregator();
      node.split = true;
    }

    friend ostream& operator<<( ostream& os, const Tree& t )
    {
      return t.preorder( os, t.nodes[ 0 ], 0 );
    }
    
    vector< Node > nodes;

  private:
    ostream& preorder( ostream& os, const Node& node, u_int level ) const
    {
      if( level )
      {
        os << std::setw( 4 * level ) << ' ';
      }
      os << node << endl;

      if( node.left > 0 ) {
        preorder( os, nodes[ node.left ], level + 1 );
      }
      if( node.right > 0 )
      {
        preorder( os, nodes[ node.right ], level + 1 );
      }

      return os;
    }
};

#endif
