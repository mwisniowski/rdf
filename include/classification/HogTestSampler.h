#ifndef RDF_HOG_TEST_SAMPLER_H
#define RDF_HOG_TEST_SAMPLER_H

#include <cvt/math/Math.h>

#include "core/Test.h"
#include "classification/HogCommon.h"
#include "classification/HogFeature.h"

template< size_t dimensions >
class HogTestSampler: public TestSamplerBase< TestType >
{
  typedef TestSamplerBase< TestType > super;

  public:
    HogTestSampler()
    {}

    HogTestSampler( const HogTestSampler& other )
    {}

    virtual ~HogTestSampler() 
    {}

    HogTestSampler& operator=( const HogTestSampler& other )
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
        HogFeature f( rand( 0, dimensions ) );
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
