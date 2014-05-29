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

#include "DataPoint.h"
#include "IDataPointCollection.h"
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

int countClasses( const IDataPointCollection& data )
{
  std::set< u_int > classes;
  for( size_t i = 0; i < data.size(); i++ )
  {
    classes.insert( data[ i ].output );
  }
  return classes.size();
}

int main(int argc, char *argv[])
{
  string file =  "~/Developer/rdf/data/supervised classification/exp1_n2.txt";
  if( argc > 1 ) {
    file = argv[ 1 ];
  }
  ifstream is( argv[ 1 ] );

  TrainingParameters params;
  params.maxDecisionLevels = 10;
  params.trees = 200;

  istream_iterator< DataPoint2f > start( is ), end;
  IDataPointCollection data( start, end );
  is.close();
  
  TrainingContext context( countClasses( data ) );
  
  // TreeTrainer trainer( context );
  // Tree classifier = trainer.trainTree( params, data );
  // cout << classifier;
  
  ForestTrainer trainer( context );
  Forest classifier = trainer.trainForest( params, data );

  int min_data = INT_MAX;
  int max_data = -INT_MIN;

  IDataPointCollection::const_iterator it = data.begin();
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
  cvt::Color color, gray( 0.5f );

  for( int row = max_data; row > min_data; row-- )
  {
    float* ptr = map.ptr();
    pt.input[ 1 ] = static_cast<float>( row );
    for( int column = min_data; column < max_data; column++ )
    {
      pt.input[ 0 ] = static_cast<float>( column );
      
      const StatisticsAggregator s = classifier.classify( pt );
      pair< u_int, float > result = s.getMax();
      if( result.first == 0 )
      {
        color.mix( gray, cvt::Color::RED, ( result.second - 0.5f ) * 2 );
      } else if (result.first == 1 )
      {
        color.mix( gray, cvt::Color::BLUE, ( result.second - 0.5f ) * 2  );
      }
      *ptr++ = color.red();
      *ptr++ = color.green();
      *ptr++ = color.blue();
      *ptr++ = color.alpha();
    }
    map++;
  }

  display( img, width, width );

  return 0;
}
