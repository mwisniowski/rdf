#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define K 9 // bins
#define SIGMA 1 // std deviation for feature extraction
#define CELLS_X 8 // number of cells in x direction
#define CELLS_Y 8 // number of cells in y direction
#define DIMENSIONS 4 * K * ( CELLS_X - 1 ) * ( CELLS_Y - 1 ) 

template< size_t dimensions >
class HogFeature;

template< size_t dimensions >
class HogTestSampler;

class HogContext;

typedef std::vector< float >                InputType;
typedef size_t                              OutputType;
typedef DataPoint< InputType, OutputType>   DataType;
typedef HogFeature< DIMENSIONS >            FeatureType;
typedef HogTestSampler< DIMENSIONS >        SamplerType;
typedef Histogram< OutputType >             StatisticsType;
typedef HogContext                          ContextType;

typedef DataPoint< InputType, OutputType >                                                 DataType;
typedef Test< FeatureType, InputType >                                                     TestType;
typedef Tree< InputType, StatisticsType, TestType >                                        TreeType;
typedef TreeTrainer< ContextType, StatisticsType, TestType, TreeType >                     TreeTrainerType;
typedef Forest< InputType, StatisticsType, TestType >                                      ForestType;
typedef ForestTrainer< ContextType, ForestType, SamplerType, TreeTrainerType, TreeType >   ForestTrainerType;

#endif
