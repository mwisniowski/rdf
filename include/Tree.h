#ifndef TREE_H
#define TREE_H

#include "Node.h"

class Tree 
{
  public:
    Tree( Node& root ) 
    {
      _nodes.push_back( root );
    }

    virtual ~Tree() 
    {}

    void setLeft( size_t parent_idx, IFeature& feature, float threshold )
    {
      _nodes.push_back( Node( feature, threshold ) );
      _nodes[ parent_idx ]._left = _nodes.size() - 1;
    }

    void setRight( size_t parent_idx, IFeature& feature, float threshold )
    {
      _nodes.push_back( Node( feature, threshold ) );
      _nodes[ parent_idx ]._right = _nodes.size() - 1;
    }

  private:
    vector< Node > _nodes;
};

#endif
