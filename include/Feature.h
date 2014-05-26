#ifndef FEATURE_H
#define FEATURE_H

#include "IFeature.h"

class Feature //: public IFeature
{
  private:
    float a;
    float b;


  public:
    Feature()
    {}

    Feature( float a, float b ) :
      a( a ),
      b( b )
    {}

    Feature( const Feature& other ) :
      a( other.a ),
      b( other.b )
    {}

    virtual ~Feature()
    {}

    float operator()( const DataPoint2f& point ) const
    {
      return a * point.input.at( 0 ) + b * point.input.at( 1 );
    }
};

#endif
