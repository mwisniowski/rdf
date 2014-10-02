#ifndef RDF_IMAGE_TEST_SAMPLER_H
#define RDF_IMAGE_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "classification/ImageCommon.h"
#include "classification/ImageFeature.h"

template< size_t channels >
class ImageTestSampler: public TestSamplerBase< TestType >
{
  typedef TestSamplerBase< TestType > super;

  public:
    ImageTestSampler()
    {}

    virtual ~ImageTestSampler() 
    {}

    void sample( std::vector< Test< FeatureType, InputType > >& tests,
        size_t num_tests ) const
    {
      tests.clear();
      for( size_t i = 0; i < num_tests; i++ )
      {
        cvt::Point2f p1( rand( 0.0f, 1.0f ), rand( 0.0f, 1.0f ) );
        cvt::Point2f p2( rand( 0.0f, 1.0f ), rand( 0.0f, 1.0f ) );
        size_t channel = cvt::Math::rand( 0, channels ) + 0.5f;
        std::vector< float > weights;
        gaussian_vector( weights, 2 );
        FeatureType f( p1, p2, weights[ 0 ], weights[ 1 ], channel );
        float threshold = rand( -255.0f, 255.0f );
        tests.push_back( Test< FeatureType, InputType >( f, threshold ) );
      }
    }

    static float rand( float LO, float HI )
    {
      return LO + static_cast <float>( std::rand() ) / ( static_cast <float>( RAND_MAX / ( HI - LO ) ) );
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
};

#endif
