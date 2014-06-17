#ifndef I_STATISTICS_H
#define I_STATISTICS_H

#include "DataRange.h"

template< typename D, typename S >
class IStatistics 
{
  public:
    virtual void aggregate( const DataRange< D >& range ) =0;

    virtual void aggregate( const S& s ) =0;

    virtual float getEntropy() const =0;
};

#endif
