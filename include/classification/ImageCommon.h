#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define CHANNELS 3

class ImageFeature;

template< size_t channels >
class ImageTestSampler;

typedef cvt::Image                                                            InputType;
typedef size_t                                                                OutputType;
typedef DataPoint< InputType, OutputType>                                     DataType;
typedef ImageFeature                                                          FeatureType;
typedef Histogram< OutputType >                                               StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< FeatureType, InputType, StatisticsType >                      ForestType;

#endif
