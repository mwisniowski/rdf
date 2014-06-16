cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(CVT_DEBUG REQUIRED)

###########
## Build ##
###########

set(RDF_HEADERS
  include/DataPoint.h
  include/DataRange.h
  include/Histogram.h
  include/Feature2f.h
  include/Forest.h
  include/ForestTrainer.h
  include/Tree.h
  include/TreeTrainer.h
  include/ThresholdSampler.h
)

set(RDF_SOURCES

)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${CVT_INCLUDE_DIRS}
)

ADD_EXECUTABLE( plot 
  ${RDF_HEADERS}
  ${RDF_SOURCES}
  src/plot.cpp
)
TARGET_LINK_LIBRARIES( plot
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( roc
  ${RDF_HEADERS}
  ${RDF_SOURCES}
  src/roc.cpp
)
TARGET_LINK_LIBRARIES( roc
  ${CVT_LIBRARIES}
)
