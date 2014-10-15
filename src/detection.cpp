#include <sstream>
#include <regex>

#include <cvt/gfx/Image.h>
#include <cvt/io/FileSystem.h>
#include <cvt/gfx/IExpr.h>

#include "detection/DetectionContext.h"
#include "detection/DetectionTestSampler.h"

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
    channels.resize( 16, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_UINT8 ) );

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
}

void get_data( std::vector< std::vector< cvt::Image > >& images,
    std::vector< std::vector< cvt::Recti > >& rois,
    cvt::String& idl_path )
{
  const std::regex filename_rgx( "\"([^\"]+)\": " );
  const std::regex rect_rgx( "\\((-?\\d+), (-?\\d+), (-?\\d+), (-?\\d+)\\)" );
  const std::regex coord_rgx( "-?\\d+" );
  std::vector< int > rgx_token_vector; 
  rgx_token_vector.push_back( 1 );
  rgx_token_vector.push_back( 2 );
  rgx_token_vector.push_back( 3 );
  rgx_token_vector.push_back( 4 );

  if( !idl_path.hasSuffix( "idl" ) )
  {
    std::cerr << "Please provide an IDL file" << std::endl;
    return;
  }
  cvt::String folder_path = idl_path.substring( 0, idl_path.rfind( '/' ) + 1 );

  std::string line;
  std::ifstream file( ( idl_path  ).c_str() );
  while( std::getline( file, line ) )
  {
    std::smatch filename_match;
    std::regex_search( line, filename_match, filename_rgx );
    std::string data_filename( filename_match[ 1 ] );

    cvt::Image i;
    i.load( folder_path + cvt::String( data_filename.c_str() ) );
    std::vector< cvt::Image > channels;
    extract_channels( channels, i );

    images.push_back( channels );

    std::vector< cvt::Recti > rects;
    const std::sregex_token_iterator end;
    std::sregex_token_iterator ri( line.begin(), line.end(), rect_rgx, rgx_token_vector );
    while( ri != end )
    {
      int x1 = atoi( ri->str().c_str() ); ++ri;
      int y1 = atoi( ri->str().c_str() ); ++ri;
      int x2 = atoi( ri->str().c_str() ); ++ri;
      int y2 = atoi( ri->str().c_str() ); ++ri;
      if( x2 < x1 ) std::swap( x1, x2 );
      if( y2 < y1 ) std::swap( y1, y2 );
      int width = x2 - x1;
      int height = y2 - y1;
      rects.push_back( cvt::Recti( x1, y1, width, height ) );
    }

    rois.push_back( rects );
  }
  file.close();
}


int main(int argc, char *argv[])
{
  std::cout << "##########     Starting     ##########" << std::endl;

  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    10,  //noCandidateFeatures
    10  //maxDecisionLevels
  };

  if( argc < 2 ) {
    std::cerr << "Please provide a data file";
    return 1;
  }

  if( argc > 3 ) params.tests = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_depth = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );

  std::cout << "Parameters:"   << std::endl;
  std::cout << "  tests="      << params.tests << std::endl;
  std::cout << "  max_depth="  << params.max_depth << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
  std::cout << "  path="       << argv[ 1 ] << std::endl;
  std::cout << "  test_img="   << argv[ 2 ] << std::endl;

  cvt::String path( argv[ 1 ] );

  std::cout << "Loading data" << std::endl;
  std::vector< std::vector< cvt::Image > > images;
  std::vector< std::vector< cvt::Recti > > rois;
  get_data( images, rois, path );

  SamplerType sampler;
  DetectionContext context( params, images, rois );

  std::cout << "Training" << std::endl;
  ForestType forest;
  ForestTrainerType::train( forest, context, sampler, true );
  cvt::String s;

  std::cout << "Testing" << std::endl;

  cvt::String image_path( argv[ 2 ] );
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
      const InputType in = { channels, x, y };
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
  std::cout << "Max peak: " << max_peak << std::endl;

  for( size_t y = 0; y < input.height(); y++ )
  {
    for( size_t x = 0; x < input.width(); x++ )
    {
      output_map( x, y ) /= max_peak;
    }
  }

  output.boxfilter( output, 3, 3 );

  cvt::Image input_gray;
  input.convert( input_gray, cvt::IFormat::GRAY_FLOAT );
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
