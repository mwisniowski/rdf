#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define RDF_FEATURE_DIMENSIONS 2

template< size_t d >
class ToyFeature;

class ToyThresholdSampler;

typedef float                                                                              InputType;
typedef size_t                                                                             OutputType;
typedef DataPoint< InputType, OutputType >                                                 DataType;
typedef ToyFeature< RDF_FEATURE_DIMENSIONS >                                               FeatureType;
typedef ToyThresholdSampler                                                                SamplerType;
typedef Histogram< OutputType >                                                            StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType, SamplerType >   TrainerType;
typedef Forest< InputType, OutputType, FeatureType, StatisticsType >                       ClassifierType;

#endif
