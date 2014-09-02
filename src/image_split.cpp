#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "helper/gnuplot_i.hpp"

#include "classification/ImageContext.h"

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
  std::cout << "##########     Starting     ##########" << std::endl;

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

  std::cout << "Parameters:" << std::endl;
  std::cout << "  features="   << params.no_candidate_features << std::endl;
  std::cout << "  thresholds=" << params.no_candate_thresholds << std::endl;
  std::cout << "  depth="      << params.max_decision_levels << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
  std::cout << "  pool_size="  << params.pool_size << std::endl;
  std::cout << "  split="      << split << std::endl;
  std::cout << "  path="       << argv[ 1 ] << std::endl;

  std::vector< DataType > data;
  cvt::String path( argv[ 1 ] );
  // std::cout << "Loading data" << std::endl;
  std::vector< cvt::String > class_labels;

  get_data( data, class_labels, path );
  size_t num_classes = class_labels.size();
  std::random_shuffle( data.begin(), data.end() );

  size_t n = data.size() * split;
  std::vector< DataType > training_data( data.begin(), data.end() - n );
  std::vector< DataType > testing_data( data.end() - n, data.end() );

  // std::cout << "Initializing context (builds lookup table)" << std::endl;
  ImageContext context( params, training_data, num_classes );
  // std::cout << "Training" << std::endl;

  ClassifierType classifier;
  TrainerType::train( classifier, context );

  // std::cout << "Classifying" << std::endl;
  std::vector< std::vector< int > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< int >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    const StatisticsType s = classifier.classify( context, testing_data[ i ] );
    confusion_matrix[ s.get_mode().first ][ testing_data[ i ].output() ]++;
  }

  float acc = 0.0f;
  for( size_t c = 0; c < num_classes; c++ )
  {
    acc += confusion_matrix[ c ][ c ];
  }
  acc /= n;

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

  std::cout << "Statistics:" << std::endl;
  std::cout << "  Accuracy: " << acc << std::endl;
  std::cout << "  MCC: " << mcc << std::endl;

  std::cout << "##########     Finished     ##########" << std::endl;

  return 0;
}
