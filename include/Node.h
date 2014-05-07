#ifndef CVT_RDF_NODE_H
#define CVT_RDF_NODE_H

#include <vector>
#include "DataPoint.h"
#include "IFeature.h"
#include "Tree.h"

using namespace std;
class Node 
{
  friend class Tree;

  public:
    Node( const IFeature& feature, float threshold ) :
      _feature( feature ),
      _threshold( threshold ),
      _left( -1 ),
      _right( -1 )
    {}

    virtual ~Node() {}


  private:
    const IFeature& _feature;
    float _threshold; 
    int _left;
    int _right;
};

#endif
