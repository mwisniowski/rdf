#ifndef DETECTION_COMMON_H
#define DETECTION_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/ForestTrainer.h"

#define CHANNELS 3
#define PATCH_SIZE 16

class DetectionFeature;

template< size_t channels >
class DetectionTestSampler;

class DetectionStatistics;

typedef cvt::Image                                                            InputType;
typedef std::pair< size_t, cvt::Vector2i >                                    OutputType;
typedef DataPoint< InputType, OutputType >                                    DataType;
typedef DetectionFeature                                                      FeatureType;
typedef DetectionTestSampler< CHANNELS >                                      SamplerType;
typedef DetectionStatistics                                                   StatisticsType;
typedef ForestTrainer< InputType, OutputType, FeatureType, StatisticsType >   TrainerType;
typedef Forest< FeatureType, InputType, StatisticsType >                      ForestType;

#endif
