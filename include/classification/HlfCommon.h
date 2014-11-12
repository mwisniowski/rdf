#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "core/Interfaces.h"
#include "core/DataPoint.h"
#include "core/Histogram.h"
#include "core/ForestTrainer.h"

#define CHANNELS 16

class ImageFeature;

template< size_t channels >
class ImageTestSampler;

class ImageContext;

typedef std::vector< cvt::Image >      InputType;
typedef size_t                         OutputType;
typedef ImageFeature                   FeatureType;
typedef ImageTestSampler< CHANNELS >   SamplerType;
typedef Histogram< OutputType >        StatisticsType;
typedef ImageContext                   ContextType;

typedef DataPoint< InputType, OutputType >                                                 DataType;
typedef Test< FeatureType, InputType >                                                     TestType;
typedef Tree< InputType, StatisticsType, TestType >                                        TreeType;
typedef TreeTrainer< ContextType, StatisticsType, TestType, TreeType >                     TreeTrainerType;
typedef Forest< InputType, StatisticsType, TestType >                                      ForestType;
typedef ForestTrainer< ContextType, ForestType, SamplerType, TreeTrainerType, TreeType >   ForestTrainerType;

#endif
