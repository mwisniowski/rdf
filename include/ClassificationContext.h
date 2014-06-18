#ifndef TRAINING_CONTEXT_H
#define TRAINING_CONTEXT_H

#include <cvt/math/Math.h>

#include "Interfaces.h"
#include "Feature.h"
#include "Histogram.h"

#define POOL_SIZE 360

class ClassificationContext : public ITrainingContext< Feature< 2 >, Histogram >
{
  public:
    typedef DataPoint< float, u_int, 2 > DataType;
    typedef Feature< 2 > FeatureType;
    typedef Histogram StatisticsType;

    const vector< Feature< 2 > >   featurePool;

    ClassificationContext( const TrainingParameters p ) :
      ITrainingContext( p ),
      featurePool( createFeaturePool() )
    {
      srand( time( 0 ) );
    }

  private:
    vector< Feature< 2 > > createFeaturePool()
    {
      // size_t pool_size = params.noCandidateFeatures * pow( 2, params.maxDecisionLevels );
      // TODO magic number
      size_t pool_size = POOL_SIZE;
      vector< Feature< 2 > > features;
      features.reserve( pool_size );
      vector< float > rv;
      for( size_t i = 0; i < pool_size; i++ )
      {
        // float angle = rand( 0.0f, TWO_PI );
        gaussianVector( rv, 2 );
        features.push_back( Feature< 2 >( rv ) );
      }
      return features;
    }

  public:


    /**
     * @brief Generate a random gaussian distributed vector using the Marsaglia Polar Method
     *
     * @param gv
     * @param dimensions
     */
    void gaussianVector( vector< float >& gv, size_t dimensions )
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

    /**
     * @brief Get random unit vectors by sampling an angle from the unit circle
     *
     * @param features
     */
    void getRandomFeatures( vector< Feature< 2 > >& features ) const
    {
      vector< size_t > indices( featurePool.size() );
      for( size_t i = 0; i < indices.size(); i++ )
      {
        indices[ i ] = i;
      }
      random_shuffle( indices.begin(), indices.end() );

      features.clear();
      for( size_t i = 0; i < params.noCandidateFeatures; i++ )
      {
        features.push_back( featurePool[ indices[ i ] ] ); 
      }
    }

    Histogram getStatisticsAggregator() const
    {
      return Histogram();
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
    float computeInformationGain( const Histogram& parent_s,
        const Histogram& left_s,
        const Histogram& right_s ) const
    {

      float H_p = parent_s.getEntropy();
      float H_l = left_s.getEntropy();
      float H_r = right_s.getEntropy();

      float fraction = left_s.n / static_cast<float>( parent_s.n );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    /**
     * @brief Criterion if a leaf should be converted to split node
     *
     * @param information_gain
     *
     * @return 
     */
    bool shouldTerminate( float information_gain ) const
    {
      // TODO Magic number
      return information_gain < 0.01f;
    }
};

#endif
