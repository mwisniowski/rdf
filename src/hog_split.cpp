#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "helper/easylogging++.h"
#include "helper/gnuplot_i.hpp"

#include "classification/HogContext.h"

_INITIALIZE_EASYLOGGINGPP

void init_logger()
{
  el::Configurations c;
  c.setGlobally( el::ConfigurationType::Format, "%datetime %level %msg" );
  c.setGlobally( el::ConfigurationType::Filename, "logs/hog_split/%datetime.log" );
  el::Loggers::reconfigureLogger( "default", c );
  el::Loggers::addFlag( el::LoggingFlag::DisableApplicationAbortOnFatalLog );
  el::Loggers::addFlag( el::LoggingFlag::LogDetailedCrashReason );
  el::Loggers::addFlag( el::LoggingFlag::ColoredTerminalOutput );
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
        cvt::Image i, g_x, g_y;
        i.load( class_data[ j ] );
        std::vector< float > feature_vector;
        FeatureType::extract_feature_vector( feature_vector, i );
        data.push_back( DataType( feature_vector, c ) );
      }
    }
  }
}

int main( int argc, char *argv[] )
{
  _START_EASYLOGGINGPP( argc, argv );
  init_logger();

  LOG(INFO) << "##########     Starting     ##########";

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

  LOG(INFO) << "Parameters:";
  LOG(INFO) << "  features="   << params.no_candidate_features;
  LOG(INFO) << "  thresholds=" << params.no_candate_thresholds;
  LOG(INFO) << "  depth="      << params.max_decision_levels;
  LOG(INFO) << "  trees="      << params.trees;
  LOG(INFO) << "  pool_size="  << params.pool_size;
  LOG(INFO) << "  split="      << split;
  LOG(INFO) << "  path="       << argv[ 1 ];

  std::vector< DataType > data;
  cvt::String path( argv[ 1 ] );
  VLOG(1) << "Loading data";
  std::vector< cvt::String > class_labels;

  get_data( data, class_labels, path );
  size_t num_classes = class_labels.size();
  std::random_shuffle( data.begin(), data.end() );

  size_t n = data.size() * split;
  std::vector< DataType > training_data( data.begin(), data.end() - n );
  std::vector< DataType > testing_data( data.end() - n, data.end() );

  VLOG(1) << "Initializing context (builds lookup table)";
  HogContext context( params, training_data, num_classes );
  VLOG(1) << "Training";

  ClassifierType classifier;
  TrainerType::train( classifier, context );

  VLOG(1) << "Classifying";
  std::vector< std::vector< size_t > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< size_t >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    const StatisticsType s = classifier.classify( context, testing_data[ i ] );
    confusion_matrix[ s.get_mode().first ][ testing_data[ i ].output() ]++;
  }

  LOG(INFO) << "Statistics:";
  std::vector< double > plot_x, plot_y;
  float acc = 0.0f;
  for( size_t c = 0; c < num_classes; c++ )
  {
    size_t true_positive = confusion_matrix[ c ][ c ];

    size_t labelled_positive = 0;
    size_t classified_positive = 0;
    for( size_t cc = 0; cc < num_classes; cc++ )
    {
      labelled_positive += confusion_matrix[ cc ][ c ];
      classified_positive += confusion_matrix[ c ][ cc ];
    }
    size_t labelled_negative = n - labelled_positive;

    size_t false_positive = classified_positive - true_positive;
    float fpr = static_cast< float >( false_positive ) / labelled_negative;
    float tpr = static_cast< float >( true_positive ) / labelled_positive;
    acc += true_positive;

    plot_x.push_back( fpr );
    plot_y.push_back( tpr );

    LOG(INFO) << "  Class " << c << ": (" << fpr << ", " << tpr << ")";
  }
  acc /= n;
  LOG(INFO) << "  Accuracy: " << acc;

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
    g.set_style("points lt 3").plot_xy( plot_x, plot_y );
  } catch( GnuplotException e )
  {
    std::cout << e.what();
  }

  LOG(INFO) << "##########     Finished     ##########";

  getchar();
  
  return 0;
}
