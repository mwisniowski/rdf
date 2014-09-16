#include <sstream>
#include <regex>

#include <cvt/gfx/Image.h>
#include <cvt/io/FileSystem.h>
#include <cvt/gfx/IExpr.h>

#include "helper/gnuplot_i.hpp"

#include "detection/DetectionContext.h"
#include "detection/DetectionTestSampler.h"

void get_data( std::vector< cvt::Image >& images,
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

    cvt::Image img;
    img.load( folder_path + cvt::String( data_filename.c_str() ) );
    img.convert( img, cvt::IFormat::RGBA_UINT8 );
    images.push_back( img );

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

  std::cout << "Loading data" << std::endl;
  std::vector< cvt::Image > images;
  std::vector< std::vector< cvt::Recti > > rois;
  get_data( images, rois, path );

  SamplerType sampler;
  DetectionContext context( params, images, rois );

  std::cout << "Training" << std::endl;
  ForestType forest;
  ForestTrainerType::train( forest, context, sampler );

  std::cout << "Testing" << std::endl;

  cvt::String image_path( argv[ 2 ] );
  cvt::Image input;
  input.load( image_path );
  input.convert( input, cvt::IFormat::RGBA_UINT8 );
  cvt::IMapScoped< const uint8_t > input_map( input );

  cvt::Image output( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT );
  cvt::IMapScoped< float > output_map( output );

  float max_peak = 0.0f;
  const size_t border = PATCH_SIZE / 2;
  for( size_t y = border; y < input.height() - border; y++ )
  {
    for( size_t x = PATCH_SIZE; x < input.width() - PATCH_SIZE; x++ )
    {
      const InputType in = { input_map, x, y };
      StatisticsType s = context.get_statistics();
      forest( s, in );
      std::pair< OutputType, float > prediction = s.predict();
      size_t predicted_class = prediction.first.first;
      if( predicted_class == 1 )
      {
        cvt::Vector2i center( cvt::Vector2i( x, y ) + prediction.first.second );
        if( center.x < output.width() && center.x >= 0 && center.y < output.height() && center.y >= 0 )
        {
          float& p_value = output_map( center.x, center.y );
          p_value += prediction.second;
          if( p_value > max_peak )
          {
            max_peak = p_value;
          }
        }
      }
    }
  }
  std::cout << "Max peak: " << max_peak << std::endl;

  size_t count = 0;
  float avg_peak = 0.0f;
  for( size_t y = border; y < input.height() - border; y++ )
  {
    for( size_t x = PATCH_SIZE; x < input.width() - PATCH_SIZE; x++ )
    {
      avg_peak += output_map( x, y );
      output_map( x, y ) /= max_peak;
      count++;
    }
  }

  std::cout << "Avg peak: " << avg_peak / count << std::endl;

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

  std::cout << "Finished" << std::endl;

  return 0;
}
