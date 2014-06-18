cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(CVT_DEBUG REQUIRED)

###########
## Build ##
###########

# set(RDF_SOURCES
#
# )

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${CVT_INCLUDE_DIRS}
)

ADD_EXECUTABLE( plot 
  src/plot.cpp
)
TARGET_LINK_LIBRARIES( plot
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( roc
  src/roc.cpp
)
TARGET_LINK_LIBRARIES( roc
  ${CVT_LIBRARIES}
)

ADD_EXECUTABLE( image
  src/image.cpp
)
TARGET_LINK_LIBRARIES( image
  ${CVT_LIBRARIES}
)
