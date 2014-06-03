#ifndef CDF_SAMPLER_H
#define CDF_SAMPLER_H

#include <cvt/math/Math.h>

#include "Feature.h"
#include "DataCollection.h"

using namespace cvt::Math;
class CDFSampler
{ 
  private:
    const Feature feature;
    const size_t n;
    const DataRange range;
    const float rmax;
 
  public:
    CDFSampler( const Feature& f, const DataRange& r ) :
      feature( f ),
      range( r ),
      rmax( RAND_MAX ),
      n( std::distance( r.start, r.end ) )
    {
      std::sort( range.start, range.end, *this );
    }

    void sample( vector<float>& thresholds, size_t size )
    {
      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        thresholds.push_back( feature( *( range.start + index ) ) );
      }
    }

    void quantiles( vector< float >& thresholds, size_t size )
    {
      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t index = ceilf( n * i / static_cast<float>( size ) ) - 1;
        thresholds.push_back( feature( *( range.start + index ) ) );
      }
    }

    void uniform( vector< float >& thresholds, size_t size )
    {
      thresholds.clear();
      thresholds.reserve( size );
      float min = feature( *range.start ),
            max = feature( *( range.end - 1) );
      for( size_t i = 0; i < size; i++ )
      {
        thresholds.push_back( rand( min, max ) );
      }
    }

    void mean( vector< float >& thresholds, size_t size )
    {
      thresholds.clear();
      thresholds.reserve( size );
      for( size_t i = 0; i < size; i++ )
      {
        size_t a = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) ),
              b = mix( static_cast<size_t>( 0 ), n-1, rand( 0.0f, 1.0f ) );
        float middle = ( feature( *( range.start + a ) ) + feature( *( range.start +b ) ) ) / 2.0f;
        thresholds.push_back( middle );
      }
    }

    bool operator()( const DataPoint2f& a, const DataPoint2f& b )
    {
      return feature( a ) < feature( b );
    }
};

#endif
