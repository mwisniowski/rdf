#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "Interfaces.h"
#include "DataPoint.h"
#include "Histogram.h"
#include "ForestTrainer.h"

class ImageFeature;

typedef cvt::String                             class_type;
typedef DataPoint< cvt::Image, class_type, 3 >  DataType;
typedef ImageFeature                            FeatureType;
typedef Histogram< class_type, DataType >       StatisticsType;

typedef ForestTrainer< DataType, FeatureType, StatisticsType > TrainerType;
typedef Forest< DataType, FeatureType, StatisticsType > ClassifierType;

#endif
