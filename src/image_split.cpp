#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "gnuplot_i.hpp"
#include "ImageCommon.h"
#include "TrainingParameters.h"
#include "DataPoint.h"
#include "ImageContext.h"
#include "ForestTrainer.h"

using namespace cvt;
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X: ", &tstruct);

    return buf;
}

size_t getData( vector< DataType >& data,
    vector< String >& class_labels, 
    String& path )
{
  if( !path.hasSuffix( "/" ) )
  {
    path += "/";
  }

  class_labels.clear();
  FileSystem::ls( path, class_labels );

  vector< String >::iterator it = class_labels.begin();
  for( ; it != class_labels.end(); ++it )
  {
    if( !it->hasPrefix( "000" ) )
    {
      class_labels.erase( it );
    }
  }

  size_t c = 0;
  for( ; c < class_labels.size(); c++ )
  {
    String p( path + class_labels[ c ] + "/" );
    if( FileSystem::isDirectory( path ) )
    {
      vector< String > class_data;
      FileSystem::filesWithExtension( p, class_data, "ppm" );
      for( size_t j = 0; j < class_data.size(); j++ )
      {
        Image i;
        i.load( class_data[ j ] );
        vector< Image > v( 3 );
        i.decompose( v[ 0 ], v[ 1 ], v[ 2 ] );
        data.push_back( DataType( v, c ) );
      }
    }
  }
  return c;
}

int main(int argc, char *argv[])
{
  TrainingParameters params = {
    1, //trees
    100,  //no_candidate_features
    100,  //no_candidate_thresholds
    15   //max_decision_levels
  };
  size_t pool_size = 3000;
  float split = 0.3;
  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) split = atof( argv[ 7 ] );

  vector< DataType > data;
  String path( argv[ 1 ] );
  cout << currentDateTime() << "Loading data" << endl;
  vector< String > class_labels;

  size_t num_classes = getData( data, class_labels, path );
  std::random_shuffle( data.begin(), data.end() );

  size_t n = data.size() * split;
  vector< DataType > training_data( data.begin(), data.end() - n );
  vector< DataType > testing_data( data.end() - n, data.end() );

  cout << currentDateTime() << "Initializing context (builds lookup table)" << endl;
  ImageContext context( params, training_data, num_classes, pool_size );
  TrainerType trainer( context );
  cout << currentDateTime() << "Training" << endl;
  ClassifierType classifer = trainer.train();

  cout << currentDateTime() << "Classifying" << endl;
  vector< vector< size_t > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( vector< size_t >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    const StatisticsType s = classifer.classify( testing_data[ i ] );
    confusion_matrix[ testing_data[ i ].output ][ s.get_mode().first ]++;
  }

  cout << "Statistics" << endl;
  vector< double > plot_x, plot_y;
  float acc = 0.0f;
  for( size_t c = 0; c < num_classes; c++ )
  {
    acc += confusion_matrix[ c ][ c ];

    size_t condition_positive = 0;
    size_t test_positive = 0;
    for( size_t cc = 0; cc < num_classes; cc++ )
    {
      condition_positive += confusion_matrix[ cc ][ c ];
      test_positive += confusion_matrix[ c ][ cc ];
    }
    size_t condition_negative = n - condition_positive;

    float fpr = ( float ) ( test_positive - confusion_matrix[ c ][ c ] ) / condition_negative;
    float tpr = ( float ) confusion_matrix[ c ][ c ] / condition_positive;

    plot_x.push_back( fpr );
    plot_y.push_back( tpr );

    cout << c << ": (" << fpr << ", " << tpr << ")" << endl;
  }
  acc /= n;
  cout << "Accuracy: " << acc << endl;

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
  } catch( GnuplotException e )
  {
    cout << e.what() << endl;
  }

  cout << currentDateTime() << "Finished" << endl;

  getchar();
  
  return 0;
}
