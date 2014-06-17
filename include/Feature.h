#ifndef FEATURE2F_H
#define FEATURE2F_H

#include "IFeature.h"

using namespace std;

class Feature : public IFeature< DataPoint< float, u_int, 2 > >
{
  private:
    vector< float > vector;

  public:
    Feature() :
      vector( 2, 0.0f )
    {
    }

    Feature( std::vector< float >& v ) :
      vector( v )
    {}

    Feature( const Feature& other ) :
      vector( other.vector )
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
    float operator()( const DataPoint< float, u_int, 2 >& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < 2; i++ )
      {
        sum += vector[ i ] * point.input[ i ];
      }
      return sum;
    }
};

#endif
