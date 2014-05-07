cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(cvt_debug REQUIRED)

###########
## Build ##
###########

set(RDF_HEADERS
  include/DataPoint.h
  include/IDataPointCollection.h
  include/IStatisticsAggregator.h
  include/IFeature.h
  include/Forest.h
  include/ForestTrainer.h
  include/Tree.h
  include/TreeTrainer.h
  include/Node.h
)

set(RDF_SOURCES

)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${CVT_INCLUDE_DIRS}
)

ADD_EXECUTABLE( main 
  ${RDF_HEADERS}
  ${RDF_SOURCES}
  src/main.cpp
)
TARGET_LINK_LIBRARIES(
  main
  ${CVT_LIBRARIES}
)
