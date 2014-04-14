cmake_minimum_required(VERSION 2.8.3)
project(CVT_RandomForest CXX C)

find_package(cvt_debug REQUIRED)

###########
## Build ##
###########

set(RT_HEADERS
  include/Instance.h
  include/CsvIterator.h
  )

set(RT_SOURCES

  )

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${CVT_INCLUDE_DIRS}
  )

ADD_EXECUTABLE( main 
  ${RT_HEADERS}
  ${RT_SOURCES}
  src/main.cpp
  )
TARGET_LINK_LIBRARIES(
  main
  ${CVT_LIBRARIES}
  )
