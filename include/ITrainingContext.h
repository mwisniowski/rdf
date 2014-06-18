#ifndef I_TRAINING_CONTEXT_H
#define I_TRAINING_CONTEXT_H

#include <vector>
#include "TrainingParameters.h"

using namespace std;

template< typename F, typename S >
class ITrainingContext
{
  public:
    const TrainingParameters params;

    ITrainingContext( const TrainingParameters& p ) :
      params( p )
    {}

    virtual void getRandomFeatures( vector< F >& features ) const =0;

    virtual S getStatisticsAggregator() const =0;

    virtual float computeInformationGain( const S& parent_s,
        const S& left_s,
        const S& right_s ) const =0;

    virtual bool shouldTerminate( float information_gain ) const =0;
};

#endif
