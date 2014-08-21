#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

#include "toy/ToyContext.h"

#include "helper/gnuplot_i.hpp"

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
  std::cout << "##########     Starting     ##########" << std::endl;

  srand( time( NULL ) );
  TrainingParameters params = {
    100, //trees
    10,  //noCandidateFeatures
    10,  //noCandidateThresholds
    10,   //maxDecisionLevels
    1000 //feature_pool_size
  };
  size_t folds = 10;

  if( argc < 2 ) {
    std::cerr << "Please provide a data file";
    return 1;
  }

  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) params.pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) folds = atoi( argv[ 7 ] );

  std::cout << "Parameters:" << std::endl;
  std::cout << "  features="   << params.no_candidate_features << std::endl;
  std::cout << "  thresholds=" << params.no_candate_thresholds << std::endl;
  std::cout << "  depth="      << params.max_decision_levels << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
  std::cout << "  pool_size="  << params.pool_size << std::endl;
  std::cout << "  folds="      << folds << std::endl;
  std::cout << "  path="       << argv[ 1 ] << std::endl;

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
    // std::cout << "Fold " << f + 1 << "/" << folds << std::endl;

    std::vector< DataType > training_data( partition_map[ 0 ], partition_map[ f ] );
    training_data.insert( training_data.end(), partition_map[ f + 1 ], partition_map.back() );
    std::vector< DataType > testing_data( partition_map[ f ], partition_map[ f + 1 ] );

    std::vector< DataType > test_data( partition_map[ f ], partition_map[ f + 1 ] );

    ToyContext context( params, training_data, num_classes );

    ClassifierType classifier;
    TrainerType::train( classifier, context );
    
    for( size_t i = 0; i < n; i++ )
    {
      const StatisticsType h = classifier.classify( context, test_data[ i ] );
      confusion_matrix[ h.get_mode().first ][ test_data[ i ].output() ]++;
    }
  }

  std::cout << "Statistics:" << std::endl;
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

    std::cout << "  Class " << class_labels[ c ] << ": (" << fpr << ", " << tpr << ")" << std::endl;
  }
  acc /= folds * n;
  std::cout << "  Accuracy: " << acc << std::endl;

  // try
  // {
  //   Gnuplot g;
  //   std::ostringstream os;
  //   os <<
  //     "features="    << params.no_candidate_features <<
  //     " thresholds=" << params.no_candate_thresholds <<
  //     " depth="      << params.max_decision_levels   <<
  //     " trees="      << params.trees                 <<
  //     " pool_size="  << params.pool_size             <<
  //     " folds="      << folds                        <<
  //     " path="       << argv[ 1 ];
  //   g.set_title( os.str() );
  //   g.set_xlabel("False positive rate");
  //   g.set_ylabel("True positive rate");
  //   g << "set size square";
  //
  //   g << "set xtics .1";
  //   g << "set ytics .1";
  //   g << "set mxtics 2";
  //   g << "set mytics 2";
  //   g.set_xrange(0,1);
  //   g.set_yrange(0,1);
  //   g.set_grid();
  //
  //   g.unset_legend();
  //   g.set_style("lines lt -1").plot_slope(1.0f,0.0f,"Random");
  //   g.set_style("points lt 3").plot_xy( plot_x, plot_y );
  //   getchar();
  // } catch( GnuplotException e )
  // {
  //   std::cout << e.what() << std::endl;
  // }

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
