#ifndef DETECTION_COMMON_H
#define DETECTION_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/ForestTrainer.h"

#define CHANNELS 1
#define PATCH_SIZE 17

template< size_t channels >
class DetectionFeature;

class DetectionStatistics;

typedef cvt::Image                                                            InputType;
typedef std::pair< size_t, cvt::Vector2i >                                    OutputType;
typedef DataPoint< InputType, OutputType >                                    DataType;
typedef DetectionFeature< CHANNELS >                                          FeatureType;
typedef DetectionStatistics                                                   StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< InputType, OutputType, FeatureType, StatisticsType >          ClassifierType;

#endif
