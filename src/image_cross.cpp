#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "classification/ImageContext.h"
#include "classification/ImageTestSampler.h"

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
    10,  //noCandidateFeatures
    10  //maxDecisionLevels
  };
  size_t folds = 10;

  if( argc > 2 ) params.tests = atoi( argv[ 2 ] );
  if( argc > 3 ) params.max_depth = atoi( argv[ 3 ] );
  if( argc > 4 ) params.trees = atoi( argv[ 4 ] );
  if( argc > 5 ) folds = atoi( argv[ 5 ] );

  std::cout << "Parameters:"   << std::endl;
  std::cout << "  tests="      << params.tests << std::endl;
  std::cout << "  max_depth="  << params.max_depth << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
  std::cout << "  folds="      << folds << std::endl;
  std::cout << "  path="       << argv[ 1 ] << std::endl;

  // std::cout << "Loading data" << std::endl;
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

    // std::cout << "Initializing context (builds lookup table)" << std::endl;
    ImageTestSampler< CHANNELS > sampler;
    ImageContext context( params, num_classes );

    // std::cout << "Training" << std::endl;
    ClassifierType classifier;
    TrainerType::train( classifier, context, sampler, training_data );
    
    // std::cout << "Classifying" << std::endl;
    for( size_t i = 0; i < n; i++ )
    {
      StatisticsType h = context.get_statistics();
      classifier.classify( h, testing_data[ i ].input() );
      confusion_matrix[ h.predict().first ][ testing_data[ i ].output() ]++;
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
