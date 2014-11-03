#ifndef TOY_COMMON_H
#define TOY_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define RDF_FEATURE_DIMENSIONS 2

template< size_t d >
class ToyFeature;

class ToyTestSampler;

class ToyContext;

typedef std::vector< float >                   InputType;
typedef size_t                                 OutputType;
typedef ToyFeature< RDF_FEATURE_DIMENSIONS >   FeatureType;
typedef ToyTestSampler                         SamplerType;
typedef Histogram< OutputType >                StatisticsType;
typedef ToyContext                             ContextType;

typedef DataPoint< InputType, OutputType >                                                 DataType;
typedef Test< FeatureType, InputType >                                                     TestType;
typedef Tree< InputType, StatisticsType, TestType >                                        TreeType;
typedef TreeTrainer< ContextType, StatisticsType, TestType, TreeType >                     TreeTrainerType;
typedef Forest< InputType, StatisticsType, TestType >                                      ForestType;
typedef ForestTrainer< ContextType, ForestType, SamplerType, TreeTrainerType, TreeType >   ForestTrainerType;

#endif
