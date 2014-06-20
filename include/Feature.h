#ifndef FEATURE2F_H
#define FEATURE2F_H

#include <vector>
#include "Interfaces.h"
#include "DataPoint.h"

template< size_t d >
class Feature : public IFeature< DataPoint< float, size_t, d > >
{
  private:
    std::vector< float > v;

  public:
    size_t id;

    Feature() :
      v( d, 0.0f ),
      id( getId() )
    {
    }

    Feature( const std::vector< float >& vec ) :
      v( vec ),
      id( getId() )
    {}

    Feature( const Feature& other ) :
      v( other.v ),
      id( other.id )
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
    float operator()( const DataPoint< float, size_t, d >& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v[ i ] * point.input[ i ];
      }
      return sum;
    }

  private:
    static size_t getId()
    {
      static size_t id( 0 );
      return id++;
    }
};

#endif
