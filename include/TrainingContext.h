#ifndef TRAINING_CONTEXT_H
#define TRAINING_CONTEXT_H

#include <cvt/math/Math.h>
#include <set>

#include "ITrainingContext.h"
#include "Feature.h"
#include "StatisticsAggregator.h"

using namespace std;
class TrainingContext// : public ITrainingContext
{
  public:
    const size_t numClasses;
    const size_t noCandidateFeatures;

  private:
    vector< Feature > features;


  public:
    TrainingContext( size_t classes, size_t candidateFeatures ) :
      numClasses( classes ),
      noCandidateFeatures( candidateFeatures )
    {
      srand(time(0));
      float max = static_cast< float >( RAND_MAX );
      features.reserve( 2 * noCandidateFeatures );
      for( size_t i = 0; i < 2 * noCandidateFeatures; i++ )
      {
        float a = rand() / max;
        float b = rand() / max;
        features.push_back( Feature( a, b ) );
      }
    }

    // const Feature getRandomFeature() const
    // {
    //   // int r = rand() % 2;
    //   // Feature f( r, !r );
    //   // return Feature( r, !r );
    //   float max = static_cast<float>(RAND_MAX);
    //   float a = rand() / max;
    //   float b = rand() / max;
    //   return Feature( a, b );
    // }
    
    const vector< Feature > sampleFeatures() const
    {
      vector< Feature > result;

      vector< size_t > indices;
      indices.reserve( 2 * noCandidateFeatures);
      for( size_t i = 0; i < 2*noCandidateFeatures; i++ )
      {
        indices.push_back( i );
      }
      std::random_shuffle( indices.begin(), indices.end() );

      for( size_t i = 0; i < noCandidateFeatures; i++ )
      {
        result.push_back( features[ indices[ i ] ] );
      }

      return result;
    }

    StatisticsAggregator getStatisticsAggregator() const
    {
      return StatisticsAggregator( numClasses );
    }

    float computeInformationGain( StatisticsAggregator& parent_s,
        StatisticsAggregator& left_s,
        StatisticsAggregator& right_s ) const
    {

      float H_p = parent_s.getEntropy();
      float H_l = left_s.getEntropy();
      float H_r = right_s.getEntropy();

      float fraction = left_s.n / 
        static_cast<float>( parent_s.n );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    bool shouldTerminate( float information_gain ) const
    {
      //TODO
      // return false;
      return information_gain < 0.01f;
    }
};

#endif
