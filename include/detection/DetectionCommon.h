#ifndef DETECTION_COMMON_H
#define DETECTION_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define CHANNELS 3

template< size_t channels >
class DetectionFeature;

typedef DataPoint< cvt::Image, size_t >                         DataType;
typedef DetectionFeature< CHANNELS >                            FeatureType;
typedef Histogram< DataType, FeatureType >                      StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >  TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >         ClassifierType;

#endif
