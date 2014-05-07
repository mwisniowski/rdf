#ifndef TRAINING_CONTEXT_H
#define TRAINING_CONTEXT_H

#include <cvt/math/Math.h>
#include <set>

#include "ITrainingContext.h"
#include "Feature.h"
#include "StatisticsAggregator.h"

class TrainingContext : public ITrainingContext
{
  public:
    TrainingContext()
    {
      srand(time(0));
    }

    const IFeature& getRandomFeature()
    {
      int r = rand() % 2;
      std::set< IFeature >::iterator it = _features.insert( Feature( r, !r ) ).first();
      return *it;
    }

    IStatisticsAggregator* getStatisticsAggregator() const
    {
      return new StatisticsAggregator();
    }

    float computeInformationGain( const IStatisticsAggregator& parent,
        const IStatisticsAggregator& left,
        const IStatisticsAggregator& right ) const
    {
      float H_p = computeEntropy( parent );
      float H_l = computeEntropy( left );
      float H_r = computeEntropy( right );

      float fraction = left.numClasses() / 
        static_cast<float>( parent.numClasses() );

      return H_p - ( fraction * H_l ) - ( ( 1.0f  - fraction ) * H_r );
    }

    bool shouldTerminate( IStatisticsAggregator& parent,
        IStatisticsAggregator& left,
        IStatisticsAggregator& right ) const
    {
      // TODO
      return true;
    }

  private:
    float computeEntropy( const IStatisticsAggregator& stats ) const
    {
      float sum = 0.0f;
      for( size_t i = 0; i < stats.numClasses(); i++ )
      {
        float p_c = stats.probability( i );
        sum += p_c * cvt::Math::log( p_c );
      }
      return -sum;
    }

    //TODO use as cache
    set< IFeature > _features;
};

#endif
