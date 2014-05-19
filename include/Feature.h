#ifndef FEATURE_H
#define FEATURE_H

#include "IFeature.h"

class Feature //: public IFeature
{
  public:
    Feature()
    {}

    Feature( float a, float b ) :
      _a( a ),
      _b( b )
    {}

    virtual ~Feature()
    {}

    float operator()( DataPoint2f& point ) const
    {
      return _a * point.input()[ 0 ] + _b * point.input()[ 1 ];
    }

  private:
    float _a;
    float _b;
};

#endif
