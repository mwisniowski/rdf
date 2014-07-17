#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

template< size_t dimensions >
class HogFeature;

typedef DataPoint< size_t, size_t >                         DataType;
typedef HogFeature< dimensions >                                FeatureType;
typedef Histogram< DataType, FeatureType >                      StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >  TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >         ClassifierType;

#endif
