#ifndef FEATURE_H
#define FEATURE_H

#include "DataPoint.h"

class Feature
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

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const DataPoint2f& point ) const
    {
      return a * point.input[ 0 ] + b * point.input[ 1 ];
    }
};

#endif
