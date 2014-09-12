#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "helper/gnuplot_i.hpp"

#include "classification/HogContext.h"
#include "classification/HogTestSampler.h"

void extract_hog_feature_vector( std::vector< float >& feature_vector, const cvt::Image& input )
{
  feature_vector.clear();
  feature_vector.resize( 4 * K * ( CELLS_X - 1 ) * ( CELLS_Y - 1 ), 0.0f );

  std::vector< float > unnormalized_feature_vector( K * CELLS_X * CELLS_Y, 0.0f );

  float kernel_data[] = { -1, 0, 1 };
  cvt::IKernel h_kernel( 3, 1, kernel_data );
  cvt::IKernel v_kernel( 1, 3, kernel_data );

  cvt::Image grayscale( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT ), 
    dx( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT ), 
    dy( input.width(), input.height(), cvt::IFormat::GRAY_FLOAT );
  input.convert( grayscale, cvt::IFormat::GRAY_FLOAT );
  grayscale.convolve( dx, cvt::IKernel::HAAR_HORIZONTAL_3 );
  grayscale.convolve( dy, cvt::IKernel::HAAR_VERTICAL_3 );

  cvt::IMapScoped< float > dx_map( dx );
  cvt::IMapScoped< float > dy_map( dy );

  // Gaussian-weighted histogram binning of gradient direction and magnitude
  float cell_width = input.width() / static_cast< float >( CELLS_X );
  float cell_height = input.height() / static_cast< float >( CELLS_Y );
  float bin_range = cvt::Math::PI / K;
  float bin_mean_base = cvt::Math::PI / ( 2 * K ); // center of 1st bin
  for( size_t y = 0; y < input.height(); y++ )
  {
    for( size_t x = 0; x < input.width(); x++ )
    {
      float g_x = dx_map( x, y );
      float g_y = dy_map( x, y );
      float magnitude = cvt::Math::sqrt( cvt::Math::sqr( g_x ) + cvt::Math::sqr( g_y ) );
      if( magnitude == 0.0f )
      {
        continue;
      }
      float angle = ( g_x > 0 ) ? ( std::atan( cvt::Math::abs( g_x ) / cvt::Math::abs( g_x ) ) ) : 0.0f;

      size_t cell_x = x / cell_width;
      size_t cell_y = y / cell_height;
      size_t offset = ( cell_y * CELLS_X + cell_x ) * K;

      size_t k = angle / cvt::Math::PI;
      float interpolation_factor = ( angle - k * bin_range ) / bin_range;
      unnormalized_feature_vector[ offset + k ] += ( 1 - interpolation_factor ) * magnitude;
      unnormalized_feature_vector[ offset + k + 1 ] += interpolation_factor * magnitude;

      // for( size_t k = 0; k < K; k++ )
      // {
      //   float bin_mean = ( 2 * k + 1 ) * bin_mean_base;
      //   float distance_to_mean = cvt::Math::exp( -( ( cvt::Math::sqr( angle - bin_mean ) ) / ( 2.0f * SIGMA * SIGMA ) ) ) / ( SIGMA * 2.506628275f );
      //   unnormalized_feature_vector[ offset + k ] += magnitude * distance_to_mean;
      // }
    }
  }

  // Block normalization
  for( size_t y = 0; y < CELLS_Y - 1; y++ )
  {
    for( size_t x = 0; x < CELLS_X - 1; x++ )
    {
      float sum = 0.0f;
      size_t cell_offset_upper = ( y * CELLS_X + x ) * K;
      size_t cell_offset_lower = ( ( y + 1 ) * CELLS_X + x ) * K;
      for( size_t i = 0; i < 2 * K; i++ )
      {
        sum += cvt::Math::sqr( unnormalized_feature_vector[ cell_offset_upper + i ] );
        sum += cvt::Math::sqr( unnormalized_feature_vector[ cell_offset_lower + i ] );
      }
      sum = cvt::Math::sqrt( sum );


      size_t block_offset_first = ( y * ( CELLS_X - 1 ) + x ) * 4 * K;
      size_t block_offset_second = block_offset_first + 2 * K;

      for( size_t i = 0; i < 2 * K; i++ )
      {
        feature_vector[ block_offset_first + i ] = unnormalized_feature_vector[ cell_offset_upper + i ] / sum;
        feature_vector[ block_offset_second + i ] = unnormalized_feature_vector[ cell_offset_lower + i ] / sum;
      }
    }
  }
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
        extract_hog_feature_vector( feature_vector, i );
        data.push_back( DataType( feature_vector, c ) );
      }
    }
  }
}

int main( int argc, char *argv[] )
{
  std::cout << "##########     Starting     ##########" << std::endl;

  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    10,  //noCandidateFeatures
    10  //maxDecisionLevels
  };
  float split = 0.3;

  if( argc < 2 ) {
    std::cerr << "Please provide a data file";
    return 1;
  }

  if( argc > 2 ) params.tests = atoi( argv[ 2 ] );
  if( argc > 3 ) params.max_depth = atoi( argv[ 3 ] );
  if( argc > 4 ) params.trees = atoi( argv[ 4 ] );
  if( argc > 5 ) split = atof( argv[ 5 ] );

  std::cout << "Parameters:"   << std::endl;
  std::cout << "  tests="      << params.tests << std::endl;
  std::cout << "  max_depth="  << params.max_depth << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
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
  SamplerType sampler;
  HogContext context( params, num_classes );
  // std::cout << "Training" << std::endl;

  ForestType forest;
  TrainerType::train( forest, context, sampler, training_data );

  // std::cout << "Classifying" << std::endl;
  std::vector< std::vector< int > > confusion_matrix;
  for( size_t i = 0; i < num_classes; i++ )
  {
    confusion_matrix.push_back( std::vector< int >( num_classes, 0 ) );
  }
  for( size_t i = 0; i < testing_data.size(); i++ )
  {
    StatisticsType s = context.get_statistics();
    forest( s, testing_data[ i ].input() );
    confusion_matrix[ s.predict().first ][ testing_data[ i ].output() ]++;
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
