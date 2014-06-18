#ifndef INTERFACES_H
#define INTERFACES_H

#include <vector>
#include "TrainingParameters.h"
#include "DataRange.h"

template< typename D >
class IFeature 
{
  public:
    virtual float operator()( const D& point ) const =0;
};

template< typename F, typename S >
class ITrainingContext
{
  public:
    const TrainingParameters params;

    ITrainingContext( const TrainingParameters& p ) :
      params( p )
    {}

    virtual void getRandomFeatures( std::vector< F >& features ) const =0;

    virtual S getStatisticsAggregator() const =0;

    virtual float computeInformationGain( const S& parent_s,
        const S& left_s,
        const S& right_s ) const =0;

    virtual bool shouldTerminate( float information_gain ) const =0;
};

template< typename D, typename S >
class IStatistics 
{
  public:
    virtual void aggregate( const DataRange< D >& range ) =0;

    virtual void aggregate( const S& s ) =0;

    virtual float getEntropy() const =0;
};



#endif
