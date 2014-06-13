#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <cvt/gui/Window.h>
#include <cvt/gui/Button.h>
#include <cvt/gui/WidgetLayout.h>
#include <cvt/gui/Moveable.h>
#include <cvt/gui/ImageView.h>
#include <cvt/gui/Application.h>

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/gfx/GFXEngineImage.h>

#include "DataPoint.h"
#include "DataCollection.h"
#include "ForestTrainer.h"
#include "TrainingParameters.h"

using namespace std;
using namespace cvt;

void display( const Image& image, size_t width, size_t height ) {
  Window w("RDF");
  
  ImageView iv;
  iv.setSize(width, height);
  iv.setImage(image);
  
  WidgetLayout wl;
  wl.setAnchoredTopBottom(0, 0);
  wl.setAnchoredLeftRight(0, 0);
  w.addWidget( &iv, wl );
  
  w.setSize( width, height );
  w.setVisible( true );
  w.update();
  
  Application::run();
}

int countClasses( const DataCollection& data )
{
  std::set< u_int > classes;
  for( size_t i = 0; i < data.size(); i++ )
  {
    classes.insert( data[ i ].output );
  }
  return classes.size();
}

// float norm( float x, size_t numClasses )
// {
//   float min = 1.0f / numClasses;
//   return ( x - min ) / ( 1.0f - min );
// }

int main(int argc, char *argv[])
{
  TrainingParameters params = {
    200, //trees
    10,  //noCandidateFeatures
    10,  //noCandidateThresholds
    10   //maxDecisionLevels
  };

  if( argc < 2 ) {
    cerr << "Please provide a data file";
    return 1;
  }
  ifstream is( argv[ 1 ] );

  if( argc > 2 ) params.noCandidateFeatures = atoi( argv[ 2 ] );
  if( argc > 3 ) params.noCandateThresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.maxDecisionLevels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );

  istream_iterator< DataPoint2f > start( is ), end;
  DataCollection data( start, end );
  is.close();
  
  TrainingContext context( countClasses( data ), params );
  
  // TreeTrainer trainer( context );
  // Tree classifier = trainer.trainTree( params, data );
  // cout << classifier;
  
  ForestTrainer trainer( context );
  Forest classifier = trainer.trainForest( params, data );

  int min_data = INT_MAX;
  int max_data = -INT_MIN;

  DataCollection::const_iterator it = data.begin();
  for( ; it != data.end(); ++it )
  {
    for( size_t i = 0; i < it->input.size(); i++ )
    {
      if( it->input.at( i )  < min_data )
      {
        min_data = it->input[ i ];
      }
      if( it->input.at( i ) > max_data )
      {
        max_data = it->input[ i ];
      }
    }
  }

  int width = max_data - min_data;

  cvt::Image img;
  img.reallocate( width, width, cvt::IFormat::RGBA_FLOAT );
  cvt::IMapScoped<float> map( img );
  DataPoint2f pt;
  pt.input.resize( 2 );
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
    pt.input[ 1 ] = static_cast<float>( row );
    for( int column = min_data; column < max_data; column++ )
    {
      pt.input[ 0 ] = static_cast<float>( column );

      const Histogram h = classifier.classify( pt );

      mix = cvt::Color::BLACK;
      float mudiness = 0.5f * h.getEntropy();
      for( size_t i = 0; i < context.numClasses; i++ )
      {
        float p = (1.0f - mudiness ) * h.probability( i );
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

  display( img, width, width );

  return 0;
}
