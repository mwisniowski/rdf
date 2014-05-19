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

    const Feature getRandomFeature()
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
      StatisticsAggregator s;
      return s;
    }

    float computeInformationGain( StatisticsAggregator& parent,
        StatisticsAggregator& left,
        StatisticsAggregator& right ) const
    {
      float H_p = parent.entropy();
      float H_l = left.entropy();
      float H_r = right.entropy();

      float fraction = left.numClasses() / 
        static_cast<float>( parent.numClasses() );

      return H_p - ( fraction * H_l ) - ( ( 1.0f  - fraction ) * H_r );
    }

    bool shouldTerminate( float information_gain ) const
    {
      //TODO
      return information_gain < 0.1f;
    }

  private:
    // set< Feature > _features;
};

#endif
