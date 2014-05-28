#ifndef TREE_H
#define TREE_H

#include <iomanip>
#include "Feature.h"
#include "StatisticsAggregator.h"

struct Node 
{
  Feature              feature;
  StatisticsAggregator statistics;
  IDataPointRange      data;
  float                threshold;
  int                  left;
  int                  right;
  bool                 split;


  // Node()
  // {
  //   init();
  // }

  Node( const Feature& feature,
      float threshold ) :
    feature( feature ),
    threshold( threshold )
  {
    init();
  }

  Node( const StatisticsAggregator& statistics,
      const IDataPointRange& range ) :
    statistics( statistics ),
    data( range ),
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

  // Node& operator=( const Node& other )
  // {
  //   if( this != &other )
  //   {
  //     feature = other.feature;
  //     statistics = other.statistics;
  //     threshold = other.threshold;
  //     left = other.left;
  //     right = other.right;
  //     data = other.data;
  //     split = other.split;
  //   }
  //   return *this;
  // }

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

    void addLeft( size_t parent, const Node& child )
    {
      nodes.push_back( child );
      nodes.at( parent ).left = nodes.size() - 1;
    }

    void addRight( size_t parent, const Node& child )
    {
      nodes.push_back( child );
      nodes.at( parent ).right = nodes.size() - 1;
    }

    const StatisticsAggregator& classify( const DataPoint2f& point ) const
    {
      // Node n = nodes[ 0 ];
      vector< Node >::const_iterator it = nodes.begin();
      // cout << "Classification trace: ";
      while( it->split )
      {
        if( it->feature( point ) < it->threshold )
        {
          // cout << "left ";
          it = nodes.begin() + it->left;
        } else {
          // cout << "right ";
          it = nodes.begin() + it->right;
        }
      }
      // cout << endl;

      // u_int c = it->statistics.maxClass();
      // return pair< u_int, float >( c, it->statistics.probability( c ) );
      return it->statistics;
    }

    void convertToSplit( size_t node_idx, float threshold, const Feature& feature )
    {
      Node& node = nodes.at( node_idx );
      node.threshold = threshold;
      node.feature = feature;
      // node.data = IDataPointCollection();
      // node.statistics = StatisticsAggregator();
      node.split = true;
    }

    friend ostream& operator<<( ostream& os, const Tree& t )
    {
      return t.preorder( os, t.nodes.at( 0 ), 0 );
    }

  private:
    ostream& preorder( ostream& os, const Node& node, int level ) const
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
