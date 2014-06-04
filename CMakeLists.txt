cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(CVT_DEBUG REQUIRED)

###########
## Build ##
###########

set(RDF_HEADERS
  include/DataPoint.h
  include/DataCollection.h
  include/Histogram.h
  include/Feature.h
  include/Forest.h
  include/ForestTrainer.h
  include/Tree.h
  include/TreeTrainer.h
  include/CDFSampler.h
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

