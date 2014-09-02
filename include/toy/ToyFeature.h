#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>

#include "toy/ToyCommon.h"

template< size_t d >
class ToyFeature: public FeatureBase< InputType, OutputType >
{
  private:
    typedef FeatureBase< InputType, OutputType > super;

  public:
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
    float operator()( const DataType& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v_[ i ] * point.input( i );
      }
      return sum;
    }

    static ToyFeature get_random_feature()
    {
      std::vector< float > v;
      gaussian_vector( v, d );
      return ToyFeature( v );
    }

  private:
    std::vector< float > v_;

    static void gaussian_vector( std::vector< float >& gv, size_t dimensions )
    {
      gv.clear();
      for( size_t i = 0; i < dimensions; i+=2 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
        gv.push_back( v * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }

      if( dimensions % 2 == 1 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }
    }
};

#endif
