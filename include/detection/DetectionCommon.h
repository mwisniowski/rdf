#ifndef DETECTION_COMMON_H
#define DETECTION_COMMON_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/ForestTrainer.h"

#define CHANNELS 32
#define PATCH_SIZE 16
#define SAMPLE_SIZE 250

class DetectionFeature;

template< size_t channels >
class DetectionTestSampler;

class DetectionStatistics;

class DetectionContext;

typedef std::vector< cvt::Image >            InputType;
typedef std::pair< size_t, cvt::Vector2i >   OutputType;
typedef DataPoint< InputType, OutputType >   DataType;
typedef DetectionFeature                     FeatureType;
typedef DetectionTestSampler< CHANNELS >     SamplerType;
typedef DetectionStatistics                  StatisticsType;
typedef DetectionContext                     ContextType;

typedef Test< FeatureType, InputType >                                                     TestType;
typedef Tree< InputType, StatisticsType, TestType >                                        TreeType;
typedef TreeTrainer< ContextType, StatisticsType, TestType, TreeType >                     TreeTrainerType;
typedef Forest< InputType, StatisticsType, TestType >                                      ForestType;
typedef ForestTrainer< ContextType, ForestType, SamplerType, TreeTrainerType, TreeType >   ForestTrainerType;

#endif
