#include <cvt/gfx/Image.h>
#include <cvt/gfx/IExpr.h>

#include "detection/DetectionCommon.h"
#include "detection/DetectionStatistics.h"
#include "detection/DetectionFeature.h"

static inline void loadbar(unsigned int x, unsigned int n, unsigned int w = 50)
{
    if ( (x != n) && (x % (n/100+1) != 0) ) return;
 
    float ratio  =  x/(float)n;
    int   c      =  ratio * w;
 
    std::cout << std::setw(3) << (int)(ratio*100) << "% [";
    for (int x=0; x<c; x++) std::cout << "=";
    for (int x=c; x<w; x++) std::cout << " ";
    std::cout << "]\r" << std::flush;
}

void extract_channels( std::vector< cvt::Image >& channels, const cvt::Image& i )
{
    cvt::Image i_float( i.width(), i.height(), cvt::IFormat::RGBA_FLOAT );
    i.convert( i_float, cvt::IFormat::RGBA_FLOAT );
    std::vector< cvt::Image > v( 3, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT ) );
    i_float.decompose( v[ 0 ], v[ 1 ], v[ 2 ] );

    cvt::Image grayscale( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
    i_float.convert( grayscale, cvt::IFormat::GRAY_FLOAT );

    cvt::Image dx( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
    cvt::Image dy( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
    cvt::Image dxx( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
    cvt::Image dyy( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
    grayscale.convolve( dx, cvt::IKernel::HAAR_HORIZONTAL_3, cvt::IKernel::GAUSS_VERTICAL_3 );
    grayscale.convolve( dy, cvt::IKernel::GAUSS_HORIZONTAL_3, cvt::IKernel::HAAR_VERTICAL_3 );
    dx.convolve( dxx, cvt::IKernel::HAAR_HORIZONTAL_3, cvt::IKernel::GAUSS_VERTICAL_3 );
    dy.convolve( dyy, cvt::IKernel::GAUSS_HORIZONTAL_3, cvt::IKernel::HAAR_VERTICAL_3 );

    std::vector< cvt::Image > hog_like( 9, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT ) );

    cvt::IMapScoped< float > dx_map( dx );
    cvt::IMapScoped< float > dy_map( dy );
    float max_magnitude = cvt::Math::EPSILONF;
    const float PI_9 = cvt::Math::PI / 9.0f;
    for( size_t y = 0; y < grayscale.height(); y++ )
    {
      for( size_t x = 0; x < grayscale.width(); x++ )
      {
        float g_x = dx_map( x, y );
        float g_y = dy_map( x, y );
        float magnitude = cvt::Math::sqrt( cvt::Math::sqr( g_x ) + cvt::Math::sqr( g_y ) );
        if( magnitude == 0.0f )
        {
          continue;
        }
        if( magnitude > max_magnitude )
        {
          max_magnitude = magnitude;
        }
        // signed angle
        float angle = cvt::Math::atan2( cvt::Math::abs( g_y ), g_x );
        size_t k = angle / PI_9;
        k = k % 9;

        cvt::IMapScoped< float > bin_map( hog_like[ k ] );
        for( int y1 = y - 2; y1 < y + 2; y1++ )
        {
          for( int x1 = x - 2; x1 < x + 2; x1++ )
          {
            if( y1 >= 0 && y1 < grayscale.height() && x1 >= 0 && x1 < grayscale.width() )
            {
              bin_map( x1, y1 ) += magnitude;
            }
          }
        }
      }
    }

    for( size_t k = 0; k < 9; k++ )
    {
      cvt::IMapScoped< float > map( hog_like[ k ] );
      for( size_t y = 0; y < i.height(); y++ )
      {
        for( size_t x = 0; x < i.width(); x++ )
        {
          map( x, y ) /= max_magnitude;
        }
      }
    }

    channels.clear();
    channels.resize( 32, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_UINT8 ) );

    for( size_t i = 0; i < 3; i++ )
    {
      v[ i ].convert( channels[ i ], cvt::IFormat::GRAY_UINT8 );
    }

    dx.convert( channels[ 3 ], cvt::IFormat::GRAY_UINT8 );
    dy.convert( channels[ 4 ], cvt::IFormat::GRAY_UINT8 );
    dxx.convert( channels[ 5 ], cvt::IFormat::GRAY_UINT8 );
    dyy.convert( channels[ 6 ], cvt::IFormat::GRAY_UINT8 );

    for( size_t i = 0; i < hog_like.size(); i++ )
    {
      hog_like[ i ].convert( channels[ 7 + i ], cvt::IFormat::GRAY_UINT8 );
    }

    for( size_t i = 0; i < 16; i++ )
    {
      channels[ i ].dilate( channels[ 16 + i ], 3 );
      channels[ i ].erode( channels[ i ], 3 );
    }
}

void get_patch( std::vector< cvt::Image >& patch, size_t x, size_t y, const std::vector< cvt::Image >& channels )
{
  patch.clear();
  for( size_t i = 0; i < channels.size(); i++ )
  {
    cvt::Recti roi( x - PATCH_SIZE / 2, y - PATCH_SIZE / 2, PATCH_SIZE, PATCH_SIZE );
    patch.push_back( cvt::Image( channels[ i ], &roi ) ); 
  }
}

int main(int argc, char *argv[])
{
  std::cout << "##########     Starting     ##########" << std::endl;

  if( argc < 2 ) {
    std::cerr << "Please provide a data file";
    return 1;
  }

  std::cout << "Parameters:"   << std::endl;
  std::cout << "  forest_xml="   << argv[ 1 ] << std::endl;
  std::cout << "  testing_image=" << argv[ 2 ] << std::endl;

  cvt::String forest_path( argv[ 1 ] );
  cvt::String image_path( argv[ 2 ] );

  ForestType forest;
  cvt::XMLDocument d;
  d.load( forest_path );
  forest.deserialize( d.nodeByName( "Forest" ) );

  std::cout << "Testing" << std::endl;

  cvt::Image input;
  input.load( image_path );
  input.convert( input, cvt::IFormat::RGBA_UINT8 );
  std::vector< cvt::Image > channels;
  extract_channels( channels, input );

  cvt::Image output( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT );
  cvt::IMapScoped< float > output_map( output );

  float max_peak = 0.0f;
  const size_t border = PATCH_SIZE / 2;
  size_t counter = 0;
  size_t total = ( input.height() - PATCH_SIZE ) * ( input.width() - PATCH_SIZE );
  for( size_t y = border; y < input.height() - border; y++ )
  {
    for( size_t x = border; x < input.width() - border; x++ )
    {
      loadbar( counter++, total );
      InputType in;
      get_patch( in, x, y, channels );
      std::vector< const StatisticsType* > statistics;
      forest.evaluate( statistics, in );

      for( size_t i = 0; i < statistics.size(); i++ )
      {
        const StatisticsType& s = *statistics[ i ];
        if( s.probability( 1 ) < 0.5f )
        {
          continue;
        }
        const StatisticsType::VectorSetType& offsets = s.offsets();
        StatisticsType::VectorSetType::const_iterator it = offsets.begin(),
          end = offsets.end();
        float weight = s.probability( 1 ) / ( offsets.size() );
        for( ; it != end; ++it )
        {
          cvt::Vector2i center( x + it->x, y + it->y );
          if( center.x < output.width() && center.x >= 0 && center.y < output.height() && center.y >= 0 )
          {
            float& p_value = output_map( center.x, center.y );
            p_value += weight;
            if( p_value > max_peak )
            {
              max_peak = p_value;
            }
          }
        }
      }
    }
  }
  loadbar( 1, 1 );
  std::cout << std::endl;
  // std::cout << "Max peak: " << max_peak << std::endl;

  for( size_t y = 0; y < input.height(); y++ )
  {
    for( size_t x = 0; x < input.width(); x++ )
    {
      output_map( x, y ) /= max_peak;
    }
  }

  output.convolve( output, cvt::IKernel::GAUSS_VERTICAL_3, cvt::IKernel::GAUSS_HORIZONTAL_3 );

  // cvt::Image input_gray;
  // input.convert( input_gray, cvt::IFormat::GRAY_FLOAT );
  // input_gray = 1.0f - input_gray;
  // input_gray.save( "input_inv.png" );

  // cvt::Image output_inv( output );
  // output_inv = ( 1.0f - output );

  output.save( "detection.png" );
  // output_inv.save( "detection_inv.png" );
  // 
  system( "open detection.png" );

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
