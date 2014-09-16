#include <fstream>

#include <cvt/gui/Window.h>
#include <cvt/gui/Button.h>
#include <cvt/gui/WidgetLayout.h>
#include <cvt/gui/Moveable.h>
#include <cvt/gui/ImageView.h>
#include <cvt/gui/Application.h>

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/gfx/GFXEngineImage.h>
#include <cvt/io/FileSystem.h>

#include "toy/ToyContext.h"
#include "toy/ToyTestSampler.h"

void display( const cvt::Image& image, size_t width, size_t height ) {
  cvt::Window w("RDF");
  
  cvt::ImageView iv;
  iv.setSize(width, height);
  iv.setImage(image);
  
  cvt::WidgetLayout wl;
  wl.setAnchoredTopBottom(0, 0);
  wl.setAnchoredLeftRight(0, 0);
  w.addWidget( &iv, wl );
  
  w.setSize( width, height );
  w.setVisible( true );
  w.update();
  
  cvt::Application::run();
}

void get_data( std::vector< DataType >& data, std::vector< char >& class_labels, const char path[] )
{
  std::ifstream is( path );
  std::string line;
  while( std::getline( is, line ) )
  {
    std::istringstream iss( line );
    std::vector< float > v( 2 );
    char c;
    if( !( iss >> c >> v[ 0 ] >> v[ 1 ] ) )
    {
      break;
    }
    std::vector< char >::iterator it = std::find( class_labels.begin(), class_labels.end(), c );
    if( it == class_labels.end() )
    {
      class_labels.push_back( c );
      it = class_labels.end() - 1;
    }
    size_t idx = std::distance( class_labels.begin(), it );
    data.push_back( DataType( v, idx ) );
  }
  is.close();
}

int main(int argc, char *argv[])
{
  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    10,  //noCandidateFeatures
    10  //maxDecisionLevels
  };

  if( argc < 2 ) {
    std::cerr << "Please provide a data file" << std::endl;
    return 1;
  }

  bool open = false;

  if( argc > 2 ) params.tests = atoi( argv[ 2 ] );
  if( argc > 3 ) params.max_depth = atoi( argv[ 3 ] );
  if( argc > 4 ) params.trees = atoi( argv[ 4 ] );
  if( argc > 5 ) open = atoi( argv[ 5 ] );

  std::vector< DataType > data;
  std::vector< char > class_labels;
  get_data( data, class_labels, argv[ 1 ] );
  size_t num_classes = class_labels.size();

  SamplerType sampler( data );
  ContextType context( params, data, num_classes );

  ForestType forest;
  ForestTrainerType::train( forest, context, sampler );

  int min_data = INT_MAX;
  int max_data = -INT_MIN;

  std::vector< DataType >::const_iterator it = data.begin();
  for( ; it != data.end(); ++it )
  {
    for( size_t i = 0; i < it->input().size(); i++ )
    {
      if( it->input()[ i ]  < min_data )
      {
        min_data = it->input()[ i ];
      }
      if( it->input()[ i ] > max_data )
      {
        max_data = it->input()[ i ];
      }
    }
  }

  min_data = ( min_data / 100 ) * 100;
  max_data = ( max_data / 100 + 1 ) * 100;

  int width = max_data - min_data;

  cvt::Image img;
  img.reallocate( width, width, cvt::IFormat::RGBA_FLOAT );
  cvt::IMapScoped<float> map( img );
  std::vector< float > v( 2 );
  cvt::Color color, gray( 0.5f ), mix;
  cvt::Color colormap[] = {
    cvt::Color::RED,
    cvt::Color::BLUE,
    cvt::Color::GREEN,
    cvt::Color::YELLOW
  };

  for( int row = max_data; row > min_data; row-- )
  {
    float* ptr = map.ptr();
    v[ 1 ] = static_cast<float>( row );
    for( int column = min_data; column < max_data; column++ )
    {
      v[ 0 ] = static_cast<float>( column );

      StatisticsType s = context.get_statistics();
      forest( s, v );

      mix = cvt::Color::BLACK;
      float mudiness = 0.5f * s.get_entropy();
      for( size_t i = 0; i < num_classes; i++ )
      {
        float p = ( 1.0f - mudiness ) * s.probability( i );
        mix = mix + colormap[ i ] * p;
      }
      mix = mix + gray * mudiness;
      
      *ptr++ = mix.red();
      *ptr++ = mix.green();
      *ptr++ = mix.blue();
      *ptr++ = mix.alpha();
    }
    map++;
  }

  cvt::String filename( argv[ 1 ] );
  filename = filename.substring( filename.rfind( '/' ) + 1, filename.length() );
  filename = filename.substring( 0, filename.rfind( '.' ) );
  filename += ".png";

  std::stringstream ss;
  ss << params.tests << "_"
    << params.max_depth << "_"
    << params.trees;
  cvt::String dirname( ss.str().c_str() );

  cvt::String root_dir( "toy_plot_results" );
  if( !cvt::FileSystem::exists( root_dir ) )
  {
    cvt::FileSystem::mkdir( root_dir );
  }
  if( !cvt::FileSystem::exists( root_dir + "/" + dirname ) )
  {
    cvt::FileSystem::mkdir( root_dir + "/" + dirname );
  }

  cvt::String result_path = root_dir + "/" + dirname + "/" + filename;
  std::cout << dirname + "/" + filename << " " << min_data << ":" << max_data << std::endl;
  img.save( result_path );

  if( open )
  {
    system( ( cvt::String( "open " ) + result_path ).c_str() );
  }

  return 0;
}
