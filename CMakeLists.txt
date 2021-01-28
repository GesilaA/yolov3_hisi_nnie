cmake_minimum_required(VERSION 2.6)
project(yolov3_hisi_nnie)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_C_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
SET(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/hisi_sdk/include
)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/hisi_sdk/lib)
link_directories(${LIB_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_DIR)

add_library(yolov3 SHARED ${SRC_DIR})

target_link_libraries(yolov3 
    pthread dnvqe isp ive mpi nnie 
    securec upvqe VoiceEngine
    _hiae _hiawb  _hidehaze _hidrc 
    _hildci hive_common
)

add_executable(yolov3_sample ${PROJECT_SOURCE_DIR}/main.c)
target_link_libraries(yolov3_sample yolov3)
