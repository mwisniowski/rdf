#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "helper/gnuplot_i.hpp"
#include "classification/ImageContext.h"

_INITIALIZE_EASYLOGGINGPP

void init_logger()
{
  el::Configurations c;
  c.setGlobally( el::ConfigurationType::Format, "%datetime %level %msg" );
  c.setGlobally( el::ConfigurationType::Filename, "logs/image_cross/%datetime.log" );
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

  LOG(INFO) << "##########     Starting     ##########";

  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    100,  //no_candidate_features
    100,  //no_candidate_thresholds
    15,   //max_decision_levels
    1000
  };

  size_t folds = 10;
  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) params.pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) folds = atoi( argv[ 7 ] );

  LOG(INFO) << "Parameters:";
  LOG(INFO) << "  features="   << params.no_candidate_features;
  LOG(INFO) << "  thresholds=" << params.no_candate_thresholds;
  LOG(INFO) << "  depth="      << params.max_decision_levels;
  LOG(INFO) << "  trees="      << params.trees;
  LOG(INFO) << "  pool_size="  << params.pool_size;
  LOG(INFO) << "  folds="      << folds;
  LOG(INFO) << "  path="       << argv[ 1 ];

  VLOG(1) << "Loading data";
  std::vector< DataType > data;
  cvt::String path( argv[ 1 ] );
  std::vector< cvt::String > class_labels;
  get_data( data, class_labels, path );
  size_t num_classes = class_labels.size();

  std::random_shuffle( data.begin(), data.end() );
  size_t n = data.size() / folds;
  std::vector< std::vector< DataType >::iterator > partition_map;
  for( size_t f = 0; f < folds; f++ )
  {
    partition_map.push_back( data.begin() + ( f * n ) );
  }
  partition_map.push_back( data.end() );

  std::vector< std::vector< size_t > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< size_t >( num_classes, 0 ) );
  }

  float divisor = static_cast<float>( n ) / folds;
  for( size_t f = 0; f < folds; f++ )
  {
    VLOG(1) << "Fold " << f + 1 << "/" << folds;
    std::vector< DataType > training_data( partition_map[ 0 ], partition_map[ f ] );
    training_data.insert( training_data.end(), partition_map[ f + 1 ], partition_map.back() );
    std::vector< DataType > testing_data( partition_map[ f ], partition_map[ f + 1 ] );

    VLOG(1) << "Initializing context (builds lookup table)";
    ImageContext context( params, training_data, num_classes );

    VLOG(1) << "Training";
    ClassifierType classifier;
    TrainerType::train( classifier, context );
    
    VLOG(1) << "Classifying";
    for( size_t i = 0; i < n; i++ )
    {
      const StatisticsType s = classifier.classify( context, testing_data[ i ] );
      confusion_matrix[ s.get_mode().first ][ testing_data[ i ].output() ]++;
    }
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
      " folds="      << folds                        <<
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
    g.set_style("points").plot_xy( plot_x, plot_y );
  } catch( GnuplotException e )
  {
    std::cout << e.what();
  }

  LOG(INFO) << "##########     Finished     ##########";

  getchar();
  
  return 0;
}
