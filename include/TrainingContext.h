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
    TrainingContext()
    {
      srand(time(0));
    }

    const Feature getRandomFeature() const
    {
      int r = rand() % 2;
      // Feature f( r, !r );
      // pair< set< Feature >::iterator, bool > ret =
      //   _features.insert( f );
      // return *ret.first;
      return Feature( r, !r );
    }

    StatisticsAggregator getStatisticsAggregator() const
    {
      return StatisticsAggregator();
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
      return information_gain < 0.001f;
    }
};

#endif
