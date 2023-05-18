cmake_minimum_required(VERSION 3.25)
project(RandomMazeSolver)

set(CMAKE_CXX_STANDARD 23)
############################################################################################################################
# SANITIZERS
OPTION(CB_THREADSANITIZER "Enable ThreadSanitizer data race detector." ON)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CMakePushCheckState)

IF (CB_THREADSANITIZER)
    CMAKE_PUSH_CHECK_STATE(RESET)
    SET(CMAKE_REQUIRED_FLAGS "-fsanitize=thread") # Also needs to be a link flag for test to pass
    CHECK_C_COMPILER_FLAG("-fsanitize=thread" HAVE_FLAG_SANITIZE_THREAD_C)
    CHECK_CXX_COMPILER_FLAG("-fsanitize=thread" HAVE_FLAG_SANITIZE_THREAD_CXX)
    CMAKE_POP_CHECK_STATE()

    IF(HAVE_FLAG_SANITIZE_THREAD_C AND HAVE_FLAG_SANITIZE_THREAD_CXX)
        SET(THREAD_SANITIZER_FLAG "-fsanitize=thread")

        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${THREAD_SANITIZER_FLAG}")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${THREAD_SANITIZER_FLAG}")
        SET(CMAKE_CGO_LDFLAGS "${CMAKE_CGO_LDFLAGS} ${THREAD_SANITIZER_FLAG} -pthread ")
        MESSAGE(STATUS "ThreadSanitizer enabled.")
    ELSE()
        MESSAGE(FATAL_ERROR "CB_THREADSANITIZER enabled but compiler doesn't support ThreadSanitizer - cannot continue.")
    ENDIF()
ENDIF()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O1")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -g -O1")
############################################################################################################################
# EXECUTABLES CONFIGURATION
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(SOURCE_FILES main.cpp sequential/sequential_version.cpp parallel/parallel_version.cpp)
add_executable(RandomMazeSolver main.cpp
        sequential/sequential_version.cpp sequential/sequential_version.h
        sequential/maze/maze_generation.cpp sequential/maze/maze_generation.h
        sequential/maze/maze_solving.cpp sequential/maze/maze_solving.h
        parallel/parallel_version.cpp  parallel/parallel_version.h
        parallel/maze/maze_generation.cpp parallel/maze/maze_generation.h
        parallel/maze/maze_solving.cpp parallel/maze/maze_solving.h
        utils/utils.cpp utils/utils.h )
############################################################################################################################
# EXTERNAL LIBRARIES
target_include_directories(RandomMazeSolver PRIVATE external/)
############################################################################################################################
# PROFILERS
set (Gperftools_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake-modules")
find_package(Gperftools REQUIRED)
target_link_libraries(RandomMazeSolver ${GPERFTOOLS_PROFILER})
############################################################################################################################