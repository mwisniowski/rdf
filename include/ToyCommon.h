#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "Interfaces.h"
#include "DataPoint.h"
#include "Histogram.h"
#include "ForestTrainer.h"

template< size_t d >
class ToyFeature;

typedef DataPoint< float, size_t >                              DataType;
typedef ToyFeature< 2 >                                         FeatureType;
typedef Histogram< DataType, FeatureType >                      StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >  TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >         ClassifierType;

#endif
