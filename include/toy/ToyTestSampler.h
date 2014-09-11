#ifndef RDF_TOY_TEST_SAMPLER_H
#define RDF_TOY_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "toy/ToyCommon.h"
#include "toy/ToyFeature.h"

class ToyTestSampler: public TestSamplerBase< FeatureType, InputType >
{
  typedef TestSamplerBase< FeatureType, InputType > super;

  public:
    ToyTestSampler( const std::vector< DataType >& data ) :
      t_min_( FLT_MAX ),
      t_max_( -FLT_MIN )
    {
      std::vector< DataType >::const_iterator it = data.begin();
      for( ; it != data.end(); ++it )
      {
        for( size_t i = 0; i < it->input().size(); i++ )
        {
          if( it->input( i )  < t_min_ )
          {
            t_min_ = it->input( i );
          }
          if( it->input( i ) > t_max_ )
          {
            t_max_ = it->input( i );
          }
        }
      }

      float range = t_max_ - t_min_;
      t_min_ -= 0.5 * range;
      t_max_ += 0.5 * range;
    }

    ToyTestSampler( const ToyTestSampler& other ) :
      t_min_( other.t_min_ ),
      t_max_( other.t_max_ )
    {}

    virtual ~ToyTestSampler() 
    {}

    ToyTestSampler& operator=( const ToyTestSampler& other )
    {
      if( this != &other )
      {
        t_min_ = other.t_min_;
        t_max_ = other.t_max_;
      }
      return *this;
    }

    void sample( std::vector< Test< FeatureType, InputType > >& tests,
        size_t num_tests ) const
    {
      tests.clear();
      for( size_t i = 0; i < num_tests; i++ )
      {
        std::vector< InputType > v;
        gaussian_vector( v, RDF_FEATURE_DIMENSIONS );
        FeatureType f( v );
        float threshold = rand( t_min_, t_max_ );
        tests.push_back( Test< FeatureType, InputType >( f, threshold ) );
      }
    }

    static void gaussian_vector( std::vector< float >& gv, size_t dimensions )
    {
      gv.clear();
      for( size_t i = 0; i < dimensions; i+=2 )
      {
        float u, v, s;
        do {
          u = rand( -1.0f, 1.0f );
          v = rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
        gv.push_back( v * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }

      if( dimensions % 2 == 1 )
      {
        float u, v, s;
        do {
          u = rand( -1.0f, 1.0f );
          v = rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }
    }

    static float rand( float LO, float HI )
    {
      return LO + static_cast <float>( std::rand() ) / ( static_cast <float>( RAND_MAX / ( HI - LO ) ) );
    }

  private:
    float t_min_;
    float t_max_;
};

#endif
