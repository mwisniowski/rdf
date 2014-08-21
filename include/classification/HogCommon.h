#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define K 9 // bins
#define SIGMA 1 // std deviation for feature extraction
#define CELLS_X 10 // number of cells in x direction
#define CELLS_Y 10 // number of cells in y direction

template< size_t dimensions >
class HogFeature;

typedef DataPoint< float, size_t >                               DataType;
typedef HogFeature< 4 * K * ( CELLS_X - 1 ) * ( CELLS_Y - 1 ) >  FeatureType;
typedef Histogram< DataType, FeatureType >                       StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >          ClassifierType;

#endif
