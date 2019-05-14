cmake_minimum_required (VERSION 2.6)
project (biketest)

set(CMAKE_BUILD_TYPE "Debug")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "-Wno-narrowing")

add_executable(biketest test_main.cpp 
    ../src/Adafruit_ST7735.cpp 
    ../src/GFX.cpp
    ../src/AppCsc.cpp
    ../src/common.cpp
    ../src/UIMain.cpp
    ../src/icons.cpp
    ../src/UISettings.cpp
    )
target_link_libraries(biketest ${GTEST_LIBRARIES} pthread)
include_directories(. ../font ../src)
