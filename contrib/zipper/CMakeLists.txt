PROJECT(zipper)

SET(LIB_NAME zipper)

FILE(GLOB ZIPPER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/zipper/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zipper/*.h
)
SOURCE_GROUP(zipper FILES ${ZIPPER_SOURCES})

ADD_LIBRARY(${LIB_NAME} STATIC ${ZIPPER_SOURCES})

TARGET_LINK_LIBRARIES (${LIB_NAME} PUBLIC zlib)
TARGET_LINK_LIBRARIES (${LIB_NAME} PUBLIC unzip)

TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/zipper)
TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${OCTOON_PATH_DEPENDENCIES}/zlib)