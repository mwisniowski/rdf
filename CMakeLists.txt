cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(CVT_DEBUG REQUIRED )

###########
## Build ##
###########

# SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${CVT_INCLUDE_DIRS}
)

ADD_EXECUTABLE( toy_plot 
  src/toy_plot.cpp
)
TARGET_LINK_LIBRARIES( toy_plot
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( toy_stat
  src/toy_stat.cpp
)

ADD_EXECUTABLE( image_split
  src/image_split.cpp
)
TARGET_LINK_LIBRARIES( image_split
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( image_cross
  src/image_cross.cpp
)

TARGET_LINK_LIBRARIES( image_cross
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( hog_split
  src/hog_split.cpp  
)
TARGET_LINK_LIBRARIES( hog_split
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( detection
  src/detection.cpp
)
TARGET_LINK_LIBRARIES( detection
  ${CVT_LIBRARIES}
)
