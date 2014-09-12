#include <sstream>
#include <regex>

#include <cvt/gfx/Image.h>
#include <cvt/io/FileSystem.h>
#include <cvt/gfx/IExpr.h>

#include "helper/gnuplot_i.hpp"

#include "detection/DetectionContext.h"
#include "detection/DetectionTestSampler.h"

void get_data( std::vector< DataType >& data,
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

    std::cout << "Processing " << data_filename << std::endl;
    cvt::Image img;
    img.load( folder_path + cvt::String( data_filename.c_str() ) );
    img.convert( img, cvt::IFormat::RGBA_UINT8 );

    std::vector< cvt::Recti > rects;
    std::vector< cvt::Vector2i > centers;
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

      int c_x( cvt::Math::round( x1 + width / 2.0f ) );
      int c_y( cvt::Math::round( y1 + height / 2.0f ) );
      centers.push_back( cvt::Vector2i( c_x, c_y ) );
    }

    std::cout << rects.size() << " rectangles" << std::endl;

    size_t count = 0;
    const size_t border = PATCH_SIZE / 2;
    for( size_t y = border; y < img.height() - border; y++ )
    {
      for( size_t x = border; x < img.width() - border; x++ )
      {
        cvt::Recti roi( x - border, y - border, PATCH_SIZE, PATCH_SIZE );
        cvt::Image patch( img, &roi, 0 );
        std::vector< cvt::Image > patch_vector( 3 );
        patch.decompose( patch_vector[ 0 ], patch_vector[ 1 ], patch_vector[ 2 ] );

        int rect_idx = -1;
        for( size_t r = 0; r < rects.size() && rect_idx < 0; r++ )
        {
          const cvt::Recti& rect = rects[ r ];
          if( rect.contains( x, y ) )
          {
            rect_idx = r;
          }
        }
        // std::cout << "Evaluating datapoint " << count << std::endl;
        if( rect_idx < 0 )
        {
          data.push_back( DataType( patch_vector, std::make_pair( 0, cvt::Vector2i( 0, 0 ) ) ) );
        } else
        {
          data.push_back( DataType( patch_vector, std::make_pair( 1, centers[ rect_idx ] - cvt::Vector2i( x, y ) ) ) );
        }
        count++;
      }
    }
    std::cout << count << " data points" << std::endl;
  }
  file.close();
}

void extract_patches( std::vector< std::vector< cvt::Image > >& patches,
    cvt::String image_path )
{
  cvt::Image img;
  img.load( image_path );
  img.convert( img, cvt::IFormat::RGBA_UINT8 );

    const size_t border = PATCH_SIZE / 2;
    for( size_t y = border; y < img.height() - border; y++ )
    {
      for( size_t x = PATCH_SIZE; x < img.width() - PATCH_SIZE; x++ )
      {
        cvt::Recti roi( x - border, y - border, PATCH_SIZE, PATCH_SIZE );
        cvt::Image patch( img, &roi, 0 );
        std::vector< cvt::Image > patch_vector( 3 );
        patch.decompose( patch_vector[ 0 ], patch_vector[ 1 ], patch_vector[ 2 ] );

        patches.push_back( patch_vector );
      }
    }
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

  std::cout << "Loading data and extracting patches" << std::endl;
  std::vector< DataType > data;
  // get_data( data, path );

  SamplerType sampler;
  DetectionContext context( params );

  std::cout << "Training" << std::endl;
  ForestType forest;
  // TrainerType::train( forest, context, sampler, data );

  std::cout << "Testing" << std::endl;

  cvt::String image_path( argv[ 2 ] );
  cvt::Image input;
  input.load( image_path );
  input.convert( input, cvt::IFormat::RGBA_UINT8 );

  cvt::Image output( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT );
  cvt::IMapScoped< float > output_map( output );

  float max_peak = 0.0f;
  const size_t border = PATCH_SIZE / 2;
  for( size_t y = border; y < input.height() - border; y++ )
  {
    for( size_t x = PATCH_SIZE; x < input.width() - PATCH_SIZE; x++ )
    {
      cvt::Recti roi( x - border, y - border, PATCH_SIZE, PATCH_SIZE );
      cvt::Image patch( input, &roi, 0 );
      std::vector< cvt::Image > patch_vector( 3 );
      patch.decompose( patch_vector[ 0 ], patch_vector[ 1 ], patch_vector[ 2 ] );

      StatisticsType s = context.get_statistics();
      forest( s, patch_vector );
      std::pair< OutputType, float > prediction = s.predict();
      if( prediction.first.first == 1 )
      {
        cvt::Vector2i v = cvt::Vector2i( x, y ) + prediction.first.second;
        if( v.x < output.width() && v.x >= 0 && v.y < output.height() && v.y >= 0 )
        {
          float& p_value = output_map( v.x, v.y );
          p_value += prediction.second;
          if( p_value > max_peak )
          {
            max_peak = p_value;
          }
        }
      }
    }
  }

  for( size_t y = border; y < input.height() - border; y++ )
  {
    for( size_t x = PATCH_SIZE; x < input.width() - PATCH_SIZE; x++ )
    {
      output_map( x, y ) /= max_peak;
    }
  }

  // cvt::Image input_gray;
  // input.convert( input_gray, cvt::IFormat::GRAY_FLOAT );
  // input_gray = 1.0f - input_gray;
  // input_gray.save( "input_inv.png" );

  // cvt::Image output_inv( output );
  // output_inv = ( 1.0f - output );

  output.save( "detection.png" );
  // output_inv.save( "detection_inv.png" );

  std::cout << "Finished" << std::endl;

  return 0;
}
