#ifndef RDF_DETECTION_TEST_SAMPLER_H
#define RDF_DETECTION_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "detection/DetectionCommon.h"
#include "detection/DetectionFeature.h"

template< size_t channels >
class DetectionTestSampler: public TestSamplerBase< Test< FeatureType, InputType > >
{
  public:
    DetectionTestSampler()
    {}

    DetectionTestSampler( const DetectionTestSampler& other )
    {}

    virtual ~DetectionTestSampler() 
    {}

    DetectionTestSampler& operator=( const DetectionTestSampler& other )
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
        cvt::Vector2i p1( rand( 0, PATCH_SIZE ), rand( 0, PATCH_SIZE ) );
        cvt::Point2f p2( rand( 0, PATCH_SIZE ), rand( 0, PATCH_SIZE ) );
        size_t channel = rand( 0, channels );
        FeatureType f( p1, p2, channel );
        float threshold = rand( -255.0f, 255.0f );
        tests.push_back( Test< FeatureType, InputType >( f, threshold ) );
      }
    }

    static float rand( float LO, float HI )
    {
      return LO + static_cast <float>( std::rand() ) / ( static_cast <float>( RAND_MAX / ( HI - LO ) ) );
    }

    static int rand( int LO, int HI )
    {
      return static_cast< int >( rand( static_cast< float >( LO ), static_cast< float >( HI ) ) );
    }
};

#endif
