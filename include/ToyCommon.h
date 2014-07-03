#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "Interfaces.h"
#include "DataPoint.h"
#include "Histogram.h"

template< size_t d >
class ToyFeature;

typedef size_t                             class_type;
typedef DataPoint< float, class_type, 2 >  DataType;
typedef ToyFeature< 2 >                       FeatureType;
typedef Histogram< class_type, DataType >  StatisticsType;

#endif
