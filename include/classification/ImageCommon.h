#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define CHANNELS 3

template< size_t channels >
class ImageFeature;

typedef cvt::Image                                                            InputType;
typedef size_t                                                                OutputType;
typedef DataPoint< InputType, OutputType>                                     DataType;
typedef ImageFeature< CHANNELS >                                              FeatureType;
typedef Histogram< InputType, OutputType, FeatureType >                       StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< InputType, OutputType, FeatureType, StatisticsType >          ClassifierType;

#endif
