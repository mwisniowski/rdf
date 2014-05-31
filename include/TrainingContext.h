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

    TrainingContext( size_t classes, size_t noCandidateFeatures_ ) :
      numClasses( classes ),
      noCandidateFeatures( noCandidateFeatures_ )
    {
      srand(time(0));
    }

    void getRandomFeatures( vector< Feature >& features ) const
    {
      features.clear();
      features.reserve( noCandidateFeatures );
      float max = static_cast<float>(RAND_MAX);
      for( size_t i = 0; i < noCandidateFeatures; i++ )
      {
        float a = rand() / max;
        float b = rand() / max;
        features.push_back( Feature( a, b ) );
      }
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
