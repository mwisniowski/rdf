#ifndef CVT_RDF_ITRAINING_CONTEXT_H
#define CVT_RDF_ITRAINING_CONTEXT_H

#include "IFeature.h"
#include "IStatisticsAggregator.h"

class ITrainingContext
{
  public:
    virtual const IFeature& getRandomFeature() const =0;
    virtual IStatisticsAggregator* getStatisticsAggregator() const =0;
    virtual float computeInformationGain( const IStatisticsAggregator& parent,
        const IStatisticsAggregator& left,
        const IStatisticsAggregator& right ) const =0;
    virtual bool shouldTerminate( IStatisticsAggregator parent, 
        IStatisticsAggregator left, 
        IStatisticsAggregator right ) const =0;
};

#endif
