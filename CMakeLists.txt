cmake_minimum_required(VERSION 2.8.3)
project(CVT_RDF CXX C)

find_package(CVT_DEBUG REQUIRED OpenMP REQUIRED)

###########
## Build ##
###########

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
if( OPENMP_FOUND )
  message("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else()
  message("NO OPENMP SUPPORT")
endif()

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

ADD_EXECUTABLE( toy_roc
  src/toy_roc.cpp
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
