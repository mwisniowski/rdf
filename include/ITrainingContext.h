#ifndef CVT_RDF_ITRAINING_CONTEXT_H
#define CVT_RDF_ITRAINING_CONTEXT_H

#include "Feature.h"
#include "StatisticsAggregator.h"

class ITrainingContext
{
  public:
    virtual const Feature& getRandomFeature() const =0;
    virtual StatisticsAggregator getStatisticsAggregator() const =0;
    virtual float computeInformationGain( const StatisticsAggregator& parent,
        const StatisticsAggregator& left,
        const StatisticsAggregator& right ) const =0;
    virtual bool shouldTerminate( float information_gain ) const =0;
};

#endif
