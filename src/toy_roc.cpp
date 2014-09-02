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


  float avg_acc = 0.0f;
  float avg_mcc = 0.0f;
  for( size_t f = 0; f < folds; f++ )
  {
    // std::cout << "Fold " << f + 1 << "/" << folds << std::endl;

    std::vector< std::vector< int > > confusion_matrix;
    for( size_t i = 0; i < num_classes; i++ )
    {
      confusion_matrix.push_back( std::vector< int >( num_classes, 0 ) );
    }

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

    float acc = 0.0f;
    for( size_t c = 0; c < num_classes; c++ )
    {
      acc += confusion_matrix[ c ][ c ];
    }
    acc /= n;
    avg_acc += acc;

    float numerator = 0.0f;
    float denominator_lk_gf = 0.0f;
    float denominator_kl_fg = 0.0f;

    for( size_t k = 0; k < num_classes; k++ )
    {
      // numerator
      for( size_t l = 0; l < num_classes; l++ )
      {
        for( size_t m = 0; m < num_classes; m++ )
        {
          numerator += confusion_matrix[ k ][ k ] * confusion_matrix[ m ][ l ] -
            confusion_matrix[ l ][ k ] * confusion_matrix[ k ][ m ];
        }
      }

      // denominator
      float sum_lk = 0.0f;
      float sum_kl = 0.0f;
      for( size_t l = 0; l < num_classes; l++ )
      {
        sum_lk += confusion_matrix[ l ][ k ];
        sum_kl += confusion_matrix[ k ][ l ];
      }

      float sum_fg = 0.0f;
      float sum_gf = 0.0f;
      for( size_t f = 0; f < num_classes; f++ )
      {
        for( size_t g = 0; g < num_classes; g++ )
        {
          if( f != k )
          {
            sum_gf += confusion_matrix[ g ][ f ];
            sum_fg += confusion_matrix[ f ][ g ];
          }
        } 
      }

      denominator_lk_gf += sum_lk * sum_gf;
      denominator_kl_fg += sum_kl * sum_fg;
    }
    float mcc = numerator / ( cvt::Math::sqrt( denominator_lk_gf ) * cvt::Math::sqrt( denominator_kl_fg ) );
    avg_mcc += mcc;
  }

  std::cout << "Statistics:" << std::endl;
  std::cout << "  Average Accuracy: " << avg_acc / folds << std::endl;
  std::cout << "  Average MCC: " << avg_mcc / folds << std::endl;

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
