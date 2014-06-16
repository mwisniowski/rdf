#ifndef FEATURE_H
#define FEATURE_H

#include "DataPoint.h"

typedef u_int class_type;

class Feature2f
{
  private:
    float a;
    float b;


  public:
    Feature2f()
    {}

    Feature2f( float a, float b ) :
      a( a ),
      b( b )
    {}

    Feature2f( const Feature2f& other ) :
      a( other.a ),
      b( other.b )
    {}

    virtual ~Feature2f()
    {}

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const DataPoint< float, class_type, 2 >& point ) const
    {
      return a * point.input[ 0 ] + b * point.input[ 1 ];
    }
};

#endif
