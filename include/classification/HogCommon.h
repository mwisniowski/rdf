#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define K 9 // bins
#define SIGMA 1 // std deviation for feature extraction
#define CELLS_X 3 // number of cells in x direction
#define CELLS_Y 3 // number of cells in y direction

class HogFeature;

template< size_t dimensions >
class HogTestSampler;

typedef float                                                                 InputType;
typedef size_t                                                                OutputType;
typedef DataPoint< InputType, OutputType>                                     DataType;
typedef HogFeature FeatureType;
typedef HogTestSampler< 4 * K * ( CELLS_X - 1 ) * ( CELLS_Y - 1 ) >           SamplerType;
typedef Histogram< OutputType >                                               StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< FeatureType, InputType, StatisticsType >                      ForestType;

#endif
