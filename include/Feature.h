#ifndef FEATURE_H
#define FEATURE_H

#include "IFeature.h"

class Feature //: public IFeature
{
  public:
    Feature()
    {}

    Feature( float a, float b ) :
      a( a ),
      b( b )
    {}

    virtual ~Feature()
    {}

    float operator()( const DataPoint2f& point ) const
    {
      return a * point.input[ 0 ] + b * point.input[ 1 ];
    }

  private:
    float a;
    float b;
};

#endif
