#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>

#include "toy/ToyCommon.h"

template< size_t d >
class ToyFeature: public FeatureBase< InputType >
{
  private:
    typedef FeatureBase< InputType > super;

  public:
    ToyFeature()
    {}

    ToyFeature( const std::vector< float >& vec ) :
      v_( vec )
    {}

    ToyFeature( const ToyFeature& other ) :
      super( other ),
      v_( other.v_ )
    {}

    virtual ~ToyFeature()
    {}

    ToyFeature& operator=( const ToyFeature& other )
    {
      if( this != &other )
      {
        v_ = other.v_;
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
    float operator()( const InputType& input ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v_[ i ] * input[ i ];
      }
      return sum;
    }

    friend std::ostream& operator<<( std::ostream& os, const ToyFeature& feature )
    {
      int last = feature.v_.size() - 1;
      os << "[";
      for(int i = 0; i < last; i++)
        os << feature.v_[ i ] << ", ";
      os << feature.v_[ last ] << "]";
      return os;
    }


  private:
    std::vector< float > v_;
};

#endif
