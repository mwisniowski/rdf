#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "HogCommon.h"

template< size_t d >
class HogFeature: public FeatureBase< DataType >
{
  private:
    typedef FeatureBase< DataType > super;

  public:
    HogFeature( const std::vector< float >& vec ) :
      v_( vec )
    {}

    HogFeature( const HogFeature& other ) :
      super( other ),
      v_( other.v_ )
    {}

    virtual ~HogFeature()
    {}

    HogFeature& operator=( const HogFeature& other )
    {
      if( this != &other )
      {
        v_ = other.v_;
      }
      return *this;
    }

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const DataType& point ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v_[ i ] * point.input( i );
      }
      return sum;
    }

    static HogFeature get_random_feature()
    {
      std::vector< float > v;
      gaussian_vector( v, d );
      return HogFeature( v );
    }

    static void extract_feature_vector( std::vector< float >& feature_vector, const cvt::Image& input )
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
          float angle = ( g_x > 0 ) ? ( std::atan( cvt::Math::abs( g_x ) / cvt::Math::abs( g_x ) ) ) : 0.0f;

          size_t cell_x = x / cell_width;
          size_t cell_y = y / cell_height;
          size_t offset = ( cell_y * CELLS_X + cell_x ) * K;

          for( size_t k = 0; k < K; k++ )
          {
            float bin_mean = ( 2 * k + 1 ) * bin_mean_base;
            // float distance_to_mean = cvt::Math::exp( -( ( cvt::Math::sqr( angle - bin_mean ) ) / ( 2.0f * SIGMA * SIGMA ) ) ) / ( SIGMA * 2.506628275f );
            float distance_to_mean = 1.0f;
            unnormalized_feature_vector[ offset + k ] += magnitude * distance_to_mean;
          }
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

  private:
    std::vector< float > v_;

    static void gaussian_vector( std::vector< float >& gv, size_t dimensions )
    {
      gv.clear();
      for( size_t i = 0; i < dimensions; i+=2 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
        gv.push_back( v * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }

      if( dimensions % 2 == 1 )
      {
        float u, v, s;
        do {
          u = cvt::Math::rand( -1.0f, 1.0f );
          v = cvt::Math::rand( -1.0f, 1.0f );
          s = u * u + v * v;
        } while ( s >= 1 );

        gv.push_back( u * sqrtf( -2 * cvt::Math::log2( s ) / s ) );
      }
    }
};

#endif
