#include <cvt/gfx/Image.h>
#include <cvt/gfx/IExpr.h>
#include <cvt/gfx/GFXEngineImage.h>
#include <cvt/vision/features/FeatureSet.h>
#include <regex>

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

// void filterNMS( cvt::Image& output, const cvt::Image& input, int radius )
// {
//   cvt::IMapScoped< const float > in_map( input );
//   cvt::IMapScoped< float > out_map( output );
//
//
//   for( int y = 0; y < input.height(); y++ )
//   {
//     int ybegin = cvt::Math::max( 0, y - radius ),
//       yend = cvt::Math::min( y + radius, (int) input.height() );
//     for( int x = 0; x < input.width(); x++ )
//     {
//       float val = in_map( x, y );
//       int curx = cvt::Math::max( 0, x - radius ),
//           xend = cvt::Math::min( x + radius, (int) input.width() );
//       for( size_t cury = ybegin; cury < yend; cury++ )
//       {
//         for( ; curx < xend; curx++ )
//         {
//           if( in_map( curx, cury ) > val )
//           {
//             goto suppressed;
//           }
//         }
//       }
//       out_map( x, y ) = val;
// suppressed:;
//     }
//   }
// }

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

void get_images( std::vector< cvt::Image >& images,
    std::vector< std::vector< cvt::Recti > >& rois,
    std::vector< cvt::String >& filenames,
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
    rois.push_back( rects );

    cvt::String filename( data_filename.c_str() );
    filenames.push_back( filename );
    cvt::Image i;
    i.load( folder_path + filename );
    images.push_back( i );
  }
  file.close();
}
void mark( cvt::Image& image, const cvt::Vector2f& point, size_t size )
{
  cvt::GFXEngineImage gi( image );
  cvt::GFX gfx( &gi );
  gfx.color().set( 1.0f, 0.0f, 0.0f );

  cvt::Vector2i p( point.x, point.y );

  gfx.drawLine( p + cvt::Vector2i(-size,0.0f), p + cvt::Vector2i(size,0.0f) );
  gfx.drawLine( p + cvt::Vector2i(0.0f,-size), p + cvt::Vector2i(0.0f,size) );
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
  std::cout << "  testing_data=" << argv[ 2 ] << std::endl;

  cvt::String forest_path( argv[ 1 ] );
  // cvt::String image_path( argv[ 2 ] );
  cvt::String testing_data_path( argv[ 2 ] );

  std::cout << "Loading data" << std::endl;

  std::vector< cvt::Image > images;
  std::vector< std::vector< cvt::Recti > > rois;
  std::vector< cvt::String > filenames;
  get_images( images, rois, filenames, testing_data_path );

  ForestType forest;
  cvt::XMLDocument d;
  d.load( forest_path );
  forest.deserialize( d.nodeByName( "Forest" ) );

  std::cout << "Testing" << std::endl;

  cvt::String dirname( forest_path );
  dirname = dirname.substring( dirname.rfind( '/' ) + 1, dirname.length() );
  dirname = dirname.substring( 0, dirname.rfind( '.' ) );

  cvt::String root_dir( "detection_results" );
  if( !cvt::FileSystem::exists( root_dir ) )
  {
    cvt::FileSystem::mkdir( root_dir );
  }
  if( !cvt::FileSystem::exists( root_dir + "/" + dirname ) )
  {
    cvt::FileSystem::mkdir( root_dir + "/" + dirname );
  }
  if( !cvt::FileSystem::exists( root_dir + "/" + dirname + "/marked" ) )
  {
    cvt::FileSystem::mkdir( root_dir + "/" + dirname + "/marked" );
  }
  if( !cvt::FileSystem::exists( root_dir + "/" + dirname + "/hough" ) )
  {
    cvt::FileSystem::mkdir( root_dir + "/" + dirname + "/hough" );
  }

  for( size_t j = 0; j < images.size(); j++ )
  {
    std::cout << "Testing image " << j+1 << "/" << images.size() << std::endl;

    cvt::Image input;
    images[ j ].convert( input, cvt::IFormat::RGBA_UINT8 );
    std::vector< cvt::Image > channels;
    extract_channels( channels, input );

    cvt::Image output( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT );
    output.fill( cvt::Color::BLACK );
    cvt::IMapScoped< float > output_map( output );

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
          // if( s.probability( 1 ) < 0.5f )
          // {
          //   continue;
          // }
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
            }
          }
        }
      }
    }
    loadbar( 1, 1 );
    std::cout << std::endl;
    // std::cout << "Max peak: " << max_peak << std::endl;

    output.convolve( output, cvt::IKernel::GAUSS_VERTICAL_7, cvt::IKernel::GAUSS_HORIZONTAL_7 );
    output.convolve( output, cvt::IKernel::GAUSS_VERTICAL_7, cvt::IKernel::GAUSS_HORIZONTAL_7 );
    // output.boxfilter( output, 9, 9 );

    float max_peak = 0.0f;
    cvt::FeatureSet fs;
    for( size_t y = 0; y < input.height(); y++ )
    {
      for( size_t x = 0; x < input.width(); x++ )
      {
        if( output_map( x, y ) > max_peak )
        {
          max_peak = output_map( x, y );
        }
        cvt::Feature f;
        f.pt = cvt::Vector2f( x, y );
        f.score = output_map( x, y );
        fs.add( f );
      }
    }

    for( size_t y = 0; y < input.height(); y++ )
    {
      for( size_t x = 0; x < input.width(); x++ )
      {
        output_map( x, y ) /= max_peak;
        output_map( x, y ) = 1.0f - output_map( x, y );
      }
    }

    cvt::Image marked( output.width(), output.height(), cvt::IFormat::RGBA_FLOAT );
    output.convert( marked, cvt::IFormat::RGBA_FLOAT );

    cvt::GFXEngineImage gi( marked );
    cvt::GFX gfx( &gi );
    gfx.setColor( cvt::Color::RED );

    for( size_t k = 0; k < rois[ j ].size(); k++ )
    {
      const cvt::Recti& r = rois[ j ][ k ];
      gfx.drawRect( r );
      // gfx.drawLine( r.x, r.y, r.x + r.width, r.y + r.height );
      // gfx.drawLine( r.x + r.width, r.y, r.x, r.y + r.height );
    }

    fs.filterNMS( 30, true );
    float avg = 0.0f;
    for( size_t k = 0; k < fs.size(); k++ )
    {
      avg += fs[ k ].score;
    }
    avg /= fs.size();
    // std::cout << "Avg: " << avg << std::endl;

    float std_dev = 0.0f;
    for( size_t k = 0; k < fs.size(); k++ )
    {
      std_dev += ( fs[ k ].score - avg ) * ( fs[ k ].score - avg );
    }
    std_dev /= fs.size() - 1;
    // std::cout << "Sigma: " << std_dev << std::endl;

    for( size_t k = 0; k < fs.size(); k++ )
    {
      if( fs[ k ].score < avg + 5 * std_dev )
      {
        continue;
      }
      cvt::Vector2f v = fs[ k ].pt;
      gfx.drawLine( v.x - 4, v.y - 4, v.x + 4, v.y + 4 );
      gfx.drawLine( v.x + 4, v.y - 4, v.x - 4, v.y + 4 );
      // std::cout << "(" << v.x << ", " << v.y << "): " << fs[ k ].score << std::endl;
    }

    cvt::String filename = filenames[ j ];
    filename = filename.substring( filename.rfind( '/' ) + 1, filename.length() );
    cvt::String result_path = root_dir + "/" + dirname + "/" + filename;

    marked.save( root_dir + "/" + dirname + "/marked/" + filename );
    output.save( root_dir + "/" + dirname + "/hough/" + filename );
  }

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
