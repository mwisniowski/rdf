#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>

#include "ToyCommon.h"

template< size_t d >
class ToyFeature: public IFeature< DataType >
{
  private:
    std::vector< float > v;
    typedef IFeature< DataType > super;

  public:
    ToyFeature() :
      // super(),
      v( d, 0.0f )
    {
    }

    ToyFeature( const std::vector< float >& vec ) :
      // super(),
      v( vec )
    {}

    ToyFeature( const ToyFeature& other ) :
      super( other ),
      v( other.v )
    {}

    virtual ~ToyFeature()
    {}

    ToyFeature& operator=( const ToyFeature& other )
    {
      if( this != &other )
      {
        super::operator=( other );
        v = other.v;
      }
      return *this;
    }

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const DataType& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v[ i ] * point.input[ i ];
      }
      return sum;
    }
};

#endif
