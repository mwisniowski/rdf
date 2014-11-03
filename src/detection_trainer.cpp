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

OutputType get_output( const std::vector< cvt::Recti >& rois, size_t x, size_t y )
{
  for( size_t r = 0; r < rois.size(); r++ )
  {
    const cvt::Recti& rect = rois[ r ];
    if( rect.contains( x, y ) )
    {
      cvt::Vector2i center( rect.x + ( rect.width / 2 ), rect.y + ( rect.height / 2 ) );
      return OutputType( 1, center - cvt::Vector2i( x, y ) );
    }
  }
  return OutputType( 0, cvt::Vector2i( 0, 0 ) );
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

    cvt::Image i;
    i.load( folder_path + cvt::String( data_filename.c_str() ) );
    std::vector< cvt::Image > channels;
    extract_channels( channels, i );

    const int border = PATCH_SIZE / 2;
    size_t pos = 0;
    size_t neg = 0;
    while( pos + neg < SAMPLE_SIZE )
    {
      size_t x = cvt::Math::rand( border, i.width() - border );
      size_t y = cvt::Math::rand( border, i.height() - border );
      OutputType output = get_output( rects, x, y );
      if( output.first == 1 && pos < SAMPLE_SIZE / 2 )
      {
        pos++;
        InputType input;
        get_patch( input, x, y, channels );
        data.push_back( DataType( input, output ) );
      }
      if( output.first == 0 && neg < SAMPLE_SIZE / 2 )
      {
        neg++;
        InputType input;
        get_patch( input, x, y, channels );
        data.push_back( DataType( input, output ) );
      }
    }
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

  std::cout  << "Parameters:"       << std::endl;
  std::cout  << "  tests="          << params.tests      << std::endl;
  std::cout  << "  max_depth="      << params.max_depth  << std::endl;
  std::cout  << "  trees="          << params.trees      << std::endl;
  std::cout  << "  training_data="  << argv[ 1 ]         << std::endl;
  std::cout  << "  forest_xml="     << argv[ 2 ]         << std::endl;

  cvt::String training_data_path( argv[ 1 ] );

  std::cout << "Loading data" << std::endl;
  std::vector< DataType > data;
  get_data( data, training_data_path );

  SamplerType sampler;
  DetectionContext context( params, data );

  std::cout << "Training" << std::endl;
  ForestType forest;
  ForestTrainerType::train( forest, context, sampler, true );

  // Save tree to file
  cvt::XMLNode* n = forest.serialize();
  cvt::XMLDocument d;
  d.addNode( n );
  d.save( argv[ 2 ] );

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
