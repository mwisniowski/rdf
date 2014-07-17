#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

template< size_t d >
class ToyFeature;

typedef DataPoint< float, size_t >                              DataType;
typedef ToyFeature< 2 >                                         FeatureType;
typedef Histogram< DataType, FeatureType >                      StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >  TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >         ClassifierType;

#endif
