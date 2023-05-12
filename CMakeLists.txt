##
##  CMakeLists.txt
##
##  Created by Gino Bollaert on 2023-05-11.
##  Copyright Gino Bollaert. All rights reserved.
##

cmake_minimum_required(VERSION 3.18)

project(waveforms VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

add_executable(gen-sine
    tools/gen_sine.cpp
)

add_executable(curves
    tools/curves.cpp
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(lfo-tests tests/main.cpp tests/WaveTableTest.cpp)
    target_include_directories(lfo-tests
    PRIVATE
        Arduino/LFO
    )
    target_link_libraries(lfo-tests PRIVATE gtest)
endif()
