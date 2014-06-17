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
#include "DataRange.h"
#include "ForestTrainer.h"
#include "TrainingParameters.h"

#include "gnuplot_i.hpp"

using namespace std;
using namespace cvt;

typedef DataPoint< float, u_int, 2 > DataPoint2f;
typedef Feature< 2 > Feature2;

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

int countClasses( const DataRange< DataPoint2f >::collection& data )
{
  typedef DataPoint< float, u_int, 2 > DataPoint2f;
  std::set< u_int > classes;
  for( size_t i = 0; i < data.size(); i++ )
  {
    classes.insert( data[ i ].output );
  }
  return classes.size();
}

int main(int argc, char *argv[])
{
  TrainingParameters params = {
    200, //trees
    10,  //noCandidateFeatures
    10,  //noCandidateThresholds
    10   //maxDecisionLevels
  };
  size_t folds = 10;

  if( argc < 2 ) {
    cerr << "Please provide a data file";
    return 1;
  }
  ifstream is( argv[ 1 ] );

  if( argc > 2 ) params.noCandidateFeatures = atoi( argv[ 2 ] );
  if( argc > 3 ) params.noCandateThresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.maxDecisionLevels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) folds = atoi( argv[ 6 ] );

  istream_iterator< DataPoint2f > start( is ), end;
  DataRange< DataPoint2f >::collection data( start, end );
  is.close();
  std::random_shuffle( data.begin(), data.end() );
  size_t n = static_cast< float >( cvt::Math::round( data.size() / static_cast<float>( folds  ) ) );
  size_t numClasses = countClasses( data );

  vector< DataRange< DataPoint2f > > partition_map;
  for( size_t f = 0; f < folds; f++ )
  {
    DataRange< DataPoint2f >::iterator it = data.begin() + f * n;
    partition_map.push_back( DataRange< DataPoint2f >( it, it + n ) );
  }

  vector< vector< size_t > > confusion_matrix;
  for( size_t i = 0; i < numClasses; i++ )
  {
    confusion_matrix.push_back( vector< size_t >( numClasses ) );
  }

  float divisor = static_cast<float>( n ) / folds;
  for( size_t f = 0; f < folds; f++ )
  {
    DataRange< DataPoint2f >::collection training_data;
    for( size_t ff = 0; ff < folds; ff++ )
    {
      if( ff != f )
      {
        training_data.insert( training_data.end(), 
            partition_map[ ff ].start,
            partition_map[ ff ].end );
      }
    }
    DataRange< DataPoint2f > training_range( training_data.begin(), training_data.end() );

    DataRange< DataPoint2f >::collection test_data( partition_map[ f ].start, partition_map[ f ].end );
    Histogram test_data_distribution;
    test_data_distribution.aggregate( DataRange< DataPoint2f >( test_data.begin(), test_data.end() ) );

    TrainingContext context( params );
    ForestTrainer< DataPoint2f, Feature2, Histogram > trainer( context );
    Forest< DataPoint2f, Feature2, Histogram > classifier = trainer.trainForest( params, training_range );
    
    for( size_t i = 0; i < n; i++ )
    {
      const Histogram h = classifier.classify( test_data[ i ] );
      confusion_matrix[ test_data[ i ].output ][ h.getMode().first ]++;
    }
  }

  vector< double > plot_x, plot_y;
  float acc = 0.0f;
  for( size_t c = 0; c < numClasses; c++ )
  {
    acc += confusion_matrix[ c ][ c ];

    size_t condition_positive = 0;
    size_t test_positive = 0;
    for( size_t cc = 0; cc < numClasses; cc++ )
    {
      condition_positive += confusion_matrix[ cc ][ c ];
      test_positive += confusion_matrix[ c ][ cc ];
    }
    size_t condition_negative = ( folds * n ) - condition_positive;

    float fpr = ( float ) ( test_positive - confusion_matrix[ c ][ c ] ) / condition_negative;
    float tpr = ( float ) confusion_matrix[ c ][ c ] / condition_positive;

    plot_x.push_back( fpr );
    plot_y.push_back( tpr );

    cout << c << ": (" << fpr << ", " << tpr << ")" << endl;
  }
  acc /= folds * n;

  cout << "Acc: " << acc << endl;

  try
  {
    Gnuplot g;
    g.set_title("ROC");
    g.set_xlabel("False positive rate");
    g.set_ylabel("True positive rate");
    g << "set size square";

    g << "set xtics .1";
    g << "set ytics .1";
    g << "set mxtics 2";
    g << "set mytics 2";
    g.set_xrange(0,1);
    g.set_yrange(0,1);
    g.set_grid();

    g.unset_legend();
    g.set_style("lines lt -1").plot_slope(1.0f,0.0f,"Random");
    g.set_style("lines lt 0").plot_slope(0.0f,acc,"Accuracy");
    g.set_style("points").plot_xy( plot_x, plot_y );

    getchar();
  } catch( GnuplotException e )
  {
    cout << e.what() << endl;
  }

  return 0;
}
