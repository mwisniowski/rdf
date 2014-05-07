#ifndef CVT_IFEATURE_H
#define CVT_IFEATURE_H

#include "DataPoint.h"

class IFeature 
{
  public:
    virtual float operator()( DataPoint2f& point ) const =0;
};

#endif
