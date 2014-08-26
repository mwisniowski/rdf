#ifndef DETECTION_COMMON_H
#define DETECTION_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/ForestTrainer.h"
#include "detection/DetectionStatistics.h"

#define CHANNELS 1
#define PATCH_SIZE 17

template< size_t channels >
class DetectionFeature;

typedef DataPoint< cvt::Image, std::pair< size_t, cvt::Vector2i > >  DataType;
typedef DetectionFeature< CHANNELS >                                 FeatureType;
typedef DetectionStatistics< DataType, FeatureType >                 StatisticsType;
typedef ForestTrainer< DataType, FeatureType, StatisticsType >       TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType >              ClassifierType;

#endif
