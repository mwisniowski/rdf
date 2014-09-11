#ifndef RDF_IMAGE_TEST_SAMPLER_H
#define RDF_IMAGE_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "classification/ImageCommon.h"
#include "classification/ImageFeature.h"

template< size_t channels >
class ImageTestSampler: public TestSamplerBase< FeatureType, InputType >
{
  typedef TestSamplerBase< FeatureType, InputType > super;

  public:
    ImageTestSampler()
    {}

    ImageTestSampler( const ImageTestSampler& other )
    {}

    virtual ~ImageTestSampler() 
    {}

    ImageTestSampler& operator=( const ImageTestSampler& other )
    {
      if( this != &other )
      {
      }
      return *this;
    }

    void sample( std::vector< Test< FeatureType, InputType > >& tests,
        size_t num_tests ) const
    {
      tests.clear();
      for( size_t i = 0; i < num_tests; i++ )
      {
        cvt::Point2f p1( rand( 0.0f, 1.0f ), rand( 0.0f, 1.0f ) );
        cvt::Point2f p2( rand( 0.0f, 1.0f ), rand( 0.0f, 1.0f ) );
        size_t channel = cvt::Math::rand( 0, channels ) + 0.5f;
        FeatureType f( p1, p2, channel );
        float threshold = rand( -255.0f, 255.0f );
        tests.push_back( Test< FeatureType, InputType >( f, threshold ) );
      }
    }

    static float rand( float LO, float HI )
    {
      return LO + static_cast <float>( std::rand() ) / ( static_cast <float>( RAND_MAX / ( HI - LO ) ) );
    }
};

#endif
