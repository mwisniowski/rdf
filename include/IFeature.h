#ifndef I_FEATURE_H
#define I_FEATURE_H

#include "DataPoint.h"

template< typename D >
class IFeature 
{
  public:
    virtual float operator()( const D& point ) const =0;
};

#endif
