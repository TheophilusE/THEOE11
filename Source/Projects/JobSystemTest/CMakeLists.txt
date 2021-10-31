
cmake_minimum_required (VERSION 3.4.3)

# Name your project. Can be anything.
project (JobSystemTest)

# Minimal supported C++ version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set binary output directories. Changing these paths will require you adjusting your resource path configuration!
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(SOURCE
    STAR.h
    STAR.cpp

    Application/Application.h
    Application/Application.cpp

    JobSystem.h
    JobSystem.cpp
)

# Define executable name.
add_executable(JobSystemTest WIN32 ${SOURCE})

# Link to game engine library.
target_link_libraries(JobSystemTest
    Urho3D
    FiberJobSystem
)
