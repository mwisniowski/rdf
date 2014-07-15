#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

#include <cvt/gui/Window.h>
#include <cvt/gui/Button.h>
#include <cvt/gui/WidgetLayout.h>
#include <cvt/gui/Moveable.h>
#include <cvt/gui/ImageView.h>
#include <cvt/gui/Application.h>

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/gfx/GFXEngineImage.h>

#include "ToyContext.h"
#include "ForestTrainer.h"

#include "gnuplot_i.hpp"
#include "easylogging++.h"

_INITIALIZE_EASYLOGGINGPP

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
    200, //trees
    10,  //noCandidateFeatures
    10,  //noCandidateThresholds
    10,   //maxDecisionLevels
    1000
  };
  size_t folds = 10;

  if( argc < 2 ) {
    std::cerr << "Please provide a data file" << std::endl;
    return 1;
  }

  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) params.pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) folds = atoi( argv[ 7 ] );

  std::vector< DataType > data;
  std::vector< char > class_labels;
  get_data( data, class_labels, argv[ 1 ] );
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
    std::cout << "Fold " << f + 1 << "/" << folds << std::endl;

    std::vector< DataType > training_data( partition_map[ 0 ], partition_map[ f ] );
    training_data.insert( training_data.end(), partition_map[ f + 1 ], partition_map.back() );
    std::vector< DataType > testing_data( partition_map[ f ], partition_map[ f + 1 ] );

    std::vector< DataType > test_data( partition_map[ f ], partition_map[ f + 1 ] );

    ToyContext context( params, training_data, num_classes );
    TrainerType trainer( context );
    ClassifierType classifier = trainer.train();
    
    for( size_t i = 0; i < n; i++ )
    {
      const StatisticsType h = classifier.classify( test_data[ i ] );
      confusion_matrix[ test_data[ i ].output() ][ h.get_mode().first ]++;
    }
  }

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
    size_t condition_negative = ( folds * n ) - condition_positive;

    float fpr = ( float ) ( test_positive - confusion_matrix[ c ][ c ] ) / condition_negative;
    float tpr = ( float ) confusion_matrix[ c ][ c ] / condition_positive;

    plot_x.push_back( fpr );
    plot_y.push_back( tpr );

    std::cout << c << ": (" << fpr << ", " << tpr << ")" << std::endl;
  }
  acc /= folds * n;

  std::cout << "Acc: " << acc << std::endl;

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
      " path="       << argv[ 1 ];
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
    std::cout << e.what() << std::endl;
  }

  getchar();
  return 0;
}
