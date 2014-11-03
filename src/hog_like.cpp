#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "classification/HlfContext.h"
#include "classification/HlfTestSampler.h"

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
        cvt::Image i_float( i.width(), i.height(), cvt::IFormat::RGBA_FLOAT );
        i.convert( i_float, cvt::IFormat::RGBA_FLOAT );
        std::vector< cvt::Image > v( 3, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT ) );
        i_float.decompose( v[ 0 ], v[ 1 ], v[ 2 ] );

        cvt::Image grayscale( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
        i_float.convert( grayscale, cvt::IFormat::GRAY_FLOAT );

        cvt::Image dx( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
        cvt::Image dy( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
        cvt::Image dxx( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
        cvt::Image dyy( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT );
        grayscale.convolve( dx, cvt::IKernel::HAAR_HORIZONTAL_3, cvt::IKernel::GAUSS_VERTICAL_3 );
        grayscale.convolve( dy, cvt::IKernel::GAUSS_HORIZONTAL_3, cvt::IKernel::HAAR_VERTICAL_3 );
        dx.convolve( dxx, cvt::IKernel::HAAR_HORIZONTAL_3, cvt::IKernel::GAUSS_VERTICAL_3 );
        dy.convolve( dyy, cvt::IKernel::GAUSS_HORIZONTAL_3, cvt::IKernel::HAAR_VERTICAL_3 );

        std::vector< cvt::Image > hog_like( 9, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_FLOAT ) );
        
        cvt::IMapScoped< float > dx_map( dx );
        cvt::IMapScoped< float > dy_map( dy );
        float max_magnitude = cvt::Math::EPSILONF;
        const float PI_9 = cvt::Math::PI / 9.0f;
        for( size_t y = 0; y < grayscale.height(); y++ )
        {
          for( size_t x = 0; x < grayscale.width(); x++ )
          {
            float g_x = dx_map( x, y );
            float g_y = dy_map( x, y );
            float magnitude = cvt::Math::sqrt( cvt::Math::sqr( g_x ) + cvt::Math::sqr( g_y ) );
            if( magnitude == 0.0f )
            {
              continue;
            }
            if( magnitude > max_magnitude )
            {
              max_magnitude = magnitude;
            }
            // signed angle
            float angle = cvt::Math::atan2( cvt::Math::abs( g_y ), g_x );
            size_t k = angle / PI_9;
            k = k % 9;

            cvt::IMapScoped< float > bin_map( hog_like[ k ] );
            for( int y1 = y - 2; y1 < y + 2; y1++ )
            {
              for( int x1 = x - 2; x1 < x + 2; x1++ )
              {
                if( y1 >= 0 && y1 < grayscale.height() && x1 >= 0 && x1 < grayscale.width() )
                {
                  bin_map( x1, y1 ) += magnitude;
                }
              }
            }
          }
        }

        for( size_t k = 0; k < 9; k++ )
        {
          cvt::IMapScoped< float > map( hog_like[ k ] );
          for( size_t y = 0; y < i.height(); y++ )
          {
            for( size_t x = 0; x < i.width(); x++ )
            {
              map( x, y ) /= max_magnitude;
            }
          }
        }

        std::vector< cvt::Image > input( 16, cvt::Image( i.width(), i.height(), cvt::IFormat::GRAY_UINT8 ) );

        for( size_t i = 0; i < 3; i++ )
        {
          v[ i ].convert( input[ i ], cvt::IFormat::GRAY_UINT8 );
        }

        dx.convert( input[ 3 ], cvt::IFormat::GRAY_UINT8 );
        dy.convert( input[ 4 ], cvt::IFormat::GRAY_UINT8 );
        dxx.convert( input[ 5 ], cvt::IFormat::GRAY_UINT8 );
        dyy.convert( input[ 6 ], cvt::IFormat::GRAY_UINT8 );

        for( size_t i = 0; i < hog_like.size(); i++ )
        {
          hog_like[ i ].convert( input[ 7 + i ], cvt::IFormat::GRAY_UINT8 );
        }

        data.push_back( DataType( input, c ) );
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

  if( argc < 2 ) {
    std::cerr << "Please provide a data file";
    return 1;
  }

  if( argc > 2 ) params.tests = atoi( argv[ 3 ] );
  if( argc > 3 ) params.max_depth = atoi( argv[ 4 ] );
  if( argc > 4 ) params.trees = atoi( argv[ 5 ] );

  std::cout << "Parameters:"   << std::endl;
  std::cout << "  tests="         << params.tests     << std::endl;
  std::cout << "  max_depth="     << params.max_depth << std::endl;
  std::cout << "  trees="         << params.trees     << std::endl;
  std::cout << "  training_path=" << argv[ 1 ]        << std::endl;
  std::cout << "  testing_path="  << argv[ 2 ]        << std::endl;

  std::vector< DataType > training_data, testing_data;
  cvt::String path_training( argv[ 1 ] );
  cvt::String path_testing( argv[ 2 ] );
  std::cout << "Loading data" << std::endl;
  std::vector< cvt::String > class_labels;

  get_data( training_data, class_labels, path_training );
  size_t num_classes = class_labels.size();

  SamplerType sampler;
  ContextType context( params, training_data, num_classes );
  std::cout << "Training" << std::endl;

  ForestType forest;
  ForestTrainerType::train( forest, context, sampler, true );

  std::cout << "Classifying" << std::endl;
  get_data( testing_data, class_labels, path_testing );
  std::vector< std::vector< int > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< int >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    StatisticsType s = context.get_statistics();
    forest.evaluate( s, testing_data[ i ].input() );
    confusion_matrix[ s.predict().first ][ testing_data[ i ].output() ]++;
  }

  float acc = 0.0f;
  for( size_t c = 0; c < num_classes; c++ )
  {
    acc += confusion_matrix[ c ][ c ];
  }
  acc /= testing_data.size();

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
