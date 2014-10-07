#ifndef RDF_HOG_TEST_SAMPLER_H
#define RDF_HOG_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "classification/HogCommon.h"
#include "classification/HogFeature.h"

template< size_t d >
class HogTestSampler: public TestSamplerBase< TestType >
{
  public:
    HogTestSampler()
    {}

    HogTestSampler( const HogTestSampler& other )
    {}

    virtual ~HogTestSampler() 
    {}

    // HogTestSampler& operator=( const HogTestSampler& other )
    // {
    //   if( this != &other )
    //   {
    //   }
    //   return *this;
    // }

    void sample( std::vector< Test< FeatureType, InputType > >& tests,
        size_t num_tests ) const
    {
      tests.clear();
      for( size_t i = 0; i < num_tests; i++ )
      {
        InputType v;
        // gaussian_vector( v, d );
        FeatureType f( rand( 0, d ) );
        float threshold = rand( -1.0f, 1.0f );
        tests.push_back( Test< FeatureType, InputType >( f, threshold ) );
      }
    }

    static void gaussian_vector( std::vector< float >& gv, size_t dimensions )
    {
      float sum = 0.0f;
      gv.clear();
      for( size_t i = 0; i < dimensions; i+=2 )
      {
        float u, v, s;
        do 
        {
          u = rand( -1.0f, 1.0f );
          v = rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } 
        while ( s >= 1 );

        float gaussian_var = u * sqrtf( -2 * cvt::Math::log2( s ) / s );
        sum += gaussian_var;
        gv.push_back( gaussian_var );

        gaussian_var = v * sqrtf( -2 * cvt::Math::log2( s ) / s );
        sum += gaussian_var;
        gv.push_back( gaussian_var );
      }

      if( dimensions % 2 == 1 )
      {
        float u, v, s;
        do
        {
          u = rand( -1.0f, 1.0f );
          v = rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } 
        while ( s >= 1 );

        float gaussian_var = u * sqrtf( -2 * cvt::Math::log2( s ) / s );
        sum += gaussian_var;
        gv.push_back( gaussian_var );
        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }

      // Make unit vector
      sum = cvt::Math::sqrt( sum );
      for( size_t i = 0; i < gv.size(); i++ )
      {
        gv[ i ] /= sum;
      }
    }

    static float rand( float LO, float HI )
    {
      return LO + static_cast <float>( std::rand() ) / ( static_cast <float>( RAND_MAX / ( HI - LO ) ) );
    }
};

#endif
