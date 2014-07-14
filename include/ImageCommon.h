#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "Interfaces.h"
#include "DataPoint.h"
#include "Histogram.h"
#include "ForestTrainer.h"

#define CHANNELS 3

template< size_t channels >
class ImageFeature;

typedef DataPoint< cvt::Image, size_t >                         DataType;
typedef ImageFeature< CHANNELS >                                            FeatureType;
typedef Histogram< DataType, FeatureType >                      StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >  TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >         ClassifierType;

#endif
