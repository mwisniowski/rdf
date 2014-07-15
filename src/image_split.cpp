#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "easylogging++.h"
#include "gnuplot_i.hpp"

#include "ImageCommon.h"
#include "TrainingParameters.h"
#include "DataPoint.h"
#include "ImageContext.h"
#include "ForestTrainer.h"

_INITIALIZE_EASYLOGGINGPP

namespace el = easyloggingpp;
void init_logger()
{
  el::Configurations defaultConf;
  defaultConf.setToDefault();
  defaultConf.setAll(easyloggingpp::ConfigurationType::Format, "%datetime %level %log");
  el::Loggers::reconfigureAllLoggers( defaultConf );
}

void get_data( std::vector< DataType >& data,
    std::vector< cvt::String >& class_labels, 
    cvt::String& path )
{
  if( !path.hasSuffix( "/" ) )
  {
    path += "/";
  }

  class_labels.clear();
  cvt::FileSystem::ls( path, class_labels );

  std::vector< cvt::String >::iterator it = class_labels.begin();
  for( ; it != class_labels.end(); ++it )
  {
    if( !it->hasPrefix( "000" ) )
    {
      class_labels.erase( it );
    }
  }

  for( size_t c = 0 ; c < class_labels.size(); c++ )
  {
    cvt::String p( path + class_labels[ c ] + "/" );
    if( cvt::FileSystem::isDirectory( path ) )
    {
      std::vector< cvt::String > class_data;
      cvt::FileSystem::filesWithExtension( p, class_data, "ppm" );
      for( size_t j = 0; j < class_data.size(); j++ )
      {
        cvt::Image i;
        i.load( class_data[ j ] );
        std::vector< cvt::Image > v( 3 );
        i.decompose( v[ 0 ], v[ 1 ], v[ 2 ] );
        data.push_back( DataType( v, c ) );
      }
    }
  }
}

int main(int argc, char *argv[])
{
  _START_EASYLOGGINGPP( argc, argv );
  init_logger();

  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    100,  //no_candidate_features
    100,  //no_candidate_thresholds
    15,   //max_decision_levels
    3000
  };

  float split = 0.3;
  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) params.pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) split = atof( argv[ 7 ] );

  std::vector< DataType > data;
  cvt::String path( argv[ 1 ] );
  LINFO << "Loading data";
  std::vector< cvt::String > class_labels;

  get_data( data, class_labels, path );
  size_t num_classes = class_labels.size();
  std::random_shuffle( data.begin(), data.end() );

  size_t n = data.size() * split;
  std::vector< DataType > training_data( data.begin(), data.end() - n );
  std::vector< DataType > testing_data( data.end() - n, data.end() );

  LINFO << "Initializing context (builds lookup table)";
  ImageContext context( params, training_data, num_classes );
  TrainerType trainer( context );
  LINFO << "Training";
  ClassifierType classifer = trainer.train();

  LINFO << "Classifying";
  std::vector< std::vector< size_t > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< size_t >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    const StatisticsType s = classifer.classify( testing_data[ i ] );
    confusion_matrix[ testing_data[ i ].output() ][ s.get_mode().first ]++;
  }

  LINFO << "Statistics";
  std::vector< double > plot_x, plot_y;
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

    LINFO << "Class " << c << ": (" << fpr << ", " << tpr << ")";
  }
  acc /= n;
  LINFO << "Accuracy: " << acc;

  try
  {
    Gnuplot g;
    std::ostringstream os;
    os <<
      "features="    << params.no_candidate_features <<
      " thresholds=" << params.no_candate_thresholds <<
      " depth="      << params.max_decision_levels   <<
      " trees="      << params.trees                 <<
      " pool_size="  << params.pool_size             <<
      " split="      << split                        <<
      " path="       << path;
    g.set_title( os.str() );
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
    std::cout << e.what();
  }

  LINFO << "Finished";

  getchar();
  
  return 0;
}
