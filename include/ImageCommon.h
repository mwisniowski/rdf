#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

#include <cvt/gfx/Image.h>

#include "Interfaces.h"
#include "DataPoint.h"
#include "Histogram.h"

class ImageFeature;

typedef size_t                             class_type;
typedef DataPoint< float, class_type, 1 >  DataType;
typedef ImageFeature                       FeatureType;
typedef Histogram< class_type, DataType >  StatisticsType;

#endif
