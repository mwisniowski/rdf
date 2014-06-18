#ifndef FEATURE2F_H
#define FEATURE2F_H

#include "IFeature.h"
#include "DataPoint.h"

using namespace std;

template< size_t d >
class Feature : public IFeature< DataPoint< float, u_int, d > >
{
  private:
    vector< float > vector;

  public:
    Feature() :
      vector( d, 0.0f )
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
    float operator()( const DataPoint< float, u_int, d >& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += vector[ i ] * point.input[ i ];
      }
      return sum;
    }
};

#endif
