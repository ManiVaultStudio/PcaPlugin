cmake_minimum_required(VERSION 3.17)
# -----------------------------------------------------------------------------
# Project: PCA plugin testing
# -----------------------------------------------------------------------------
set(PCA_TESTS "PcaTests")

PROJECT(${PCA_TESTS})

# -----------------------------------------------------------------------------
# Set cmake flags
# -----------------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

set(PCA_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

IF( NOT DEFINED EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Define EIGEN3_INCLUDE_DIR, the directory of Eigen3")
ELSE()
	MESSAGE( STATUS "Eigen headers at ${EIGEN3_INCLUDE_DIR}")
ENDIF()
# don't use find_package (Eigen3 3.4 REQUIRED) for now since this requires more user setup

if(DEFINED ENV{CATCH2_DIR})
    set(CATCH2_DIR $ENV{CATCH2_DIR} CACHE PATH  "Path to HDILib_slim")
endif()

find_package(Catch2 3 REQUIRED PATH ${CATCH2_DIR})

IF( ${Catch2_FOUND} )
    MESSAGE( STATUS "Catch2 found at ${Catch2_DIR}")
ENDIF()
# don't use find_package (Catch2 3 REQUIRED) for now since this requires more user setup

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(SOURCES
    test.cpp
)

source_group( Tests FILES ${SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------

add_executable(${PCA_TESTS} ${SOURCES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------

# Include pca 
target_include_directories(${PCA_TESTS} PRIVATE "${PCA_PLUGIN_DIR}/src")

# Include external headers
target_include_directories(${PCA_TESTS} PRIVATE "${EIGEN3_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target link directories
# -----------------------------------------------------------------------------
# linking against catch will add the proper include path
target_link_libraries(${PCA_TESTS} PRIVATE Catch2::Catch2WithMain)

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------

# Request C++17
target_compile_features(${PCA_TESTS} PRIVATE cxx_std_17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")	# strict standard conformance
    target_compile_options(${PCA_TESTS} PRIVATE /bigobj)	# for Eigen
endif(MSVC)
