#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

template< size_t d >
class ToyFeature;

typedef float                                                                 InputType;
typedef size_t                                                                OutputType;
typedef DataPoint< InputType, OutputType >                                    DataType;
typedef ToyFeature< 2 >                                                       FeatureType;
typedef Histogram< InputType, OutputType, FeatureType >                       StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< InputType, OutputType, FeatureType, StatisticsType >          ClassifierType;

#endif
