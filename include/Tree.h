#ifndef TREE_H
#define TREE_H

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
    _feature( feature ),
    _threshold( threshold )
  {
    init();
  }

  Node( const StatisticsAggregator& statistics,
      const IDataPointCollection& data ) :
    _statistics( statistics ),
    _data( data )
  {
    init();
  }

  Node( const Node& other ) :
    _feature( other._feature ),
    _statistics( other._statistics ),
    _threshold( other._threshold ),
    _left( other._left ),
    _right( other._right ),
    _data( other._data )
  {}

  Node& operator=( const Node& other )
  {
    if( this != &other )
    {
      _feature = other._feature;
      _statistics = other._statistics;
      _threshold = other._threshold;
      _left = other._left;
      _right = other._right;
      _data = other._data;
    }
    return *this;
  }

  virtual ~Node() 
  {}

  void init()
  {
    _left = -1;
    _right = -1;
  }

  bool isSplit()
  {
    return _left > 0 && _right > 0;
  }

  Feature              _feature;
  StatisticsAggregator _statistics;
  IDataPointCollection _data; //TODO Inplace partitioning
  float                _threshold;
  int                  _left;
  int                  _right;
};

class Tree 
{
  public:
    Tree( Node& root ) 
    {
      _nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    void addLeft( Node& parent, Node& child )
    {
      _nodes.push_back( child );
      parent._left = _nodes.size() - 1;
    }

    void addRight( Node& parent, Node& child )
    {
      _nodes.push_back( child );
      parent._right = _nodes.size() - 1;
    }

    pair< u_int, float > classify( DataPoint2f& point )
    {
      Node n = _nodes[ 0 ];
      while( n.isSplit() )
      {
        if( n._feature( point ) < n._threshold )
        {
          n = _nodes[ n._left ];
        } else {
          n = _nodes[ n._right ];
        }
      }

      u_int c = n._statistics.maxClass();
      return pair< u_int, float >( c, n._statistics.probability( c ) );
    }

  private:
    vector< Node > _nodes;
};

#endif
