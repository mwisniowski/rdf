#ifndef TOY_CONTEXT_H
#define TOY_CONTEXT_H

#include <cvt/math/Math.h>
#include <map>

#include "ToyCommon.h"
#include "ToyFeature.h"

#define POOL_SIZE 360

class ToyContext : public ITrainingContext< DataType, FeatureType, StatisticsType >
{
  private:
    typedef ITrainingContext< DataType, FeatureType, StatisticsType > super;

  private:
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

    static std::vector< FeatureType > pool_init( size_t pool_size )
    {
      std::vector< FeatureType > features;
      features.reserve( pool_size );
      std::vector< float > rv;
      for( size_t id = 0; id < pool_size; id++ )
      {
        gaussian_vector( rv, 2 );
        FeatureType f( rv );
        f.id = id;
        features.push_back( f );
      }
      return features;
    }

  public:
    ToyContext( const TrainingParameters& params,
        const std::vector< DataType >& data,
        size_t num_classes ) :
      super( params, data, pool_init, num_classes )
    {
    }

    virtual ~ToyContext()
    {
    }

    StatisticsType get_statistics() const
    {
      return StatisticsType( *this );
    }

    /**
     * @brief Compute information gain by subtracting the sum of weighted child-entropies
     * from parent entropy
     *
     * @param parent_s
     * @param left_s
     * @param right_s
     *
     * @return 
     */
    float compute_information_gain( const StatisticsType& parent_statistics,
        const StatisticsType& left_statistics,
        const StatisticsType& right_statistics ) const
    {
      float H_p = parent_statistics.get_entropy();
      float H_l = left_statistics.get_entropy();
      float H_r = right_statistics.get_entropy();

      float fraction = left_statistics.n / static_cast<float>( parent_statistics.n );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    /**
     * @brief Criterion if a leaf should be converted to split node
     *
     * @param information_gain
     *
     * @return 
     */
    bool should_terminate( float information_gain ) const
    {
      // TODO Magic number
      return information_gain < 0.01f;
    }
};

#endif
