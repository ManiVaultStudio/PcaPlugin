cmake_minimum_required(VERSION 3.21)

option(USE_OPENMP "Use OpenMP - by default ON" ON)

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
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

find_package(OpenMP)

set(PCA_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# EIGEN3
set(Eigen3_DIR "${PCA_PLUGIN_DIR}/third_party/eigen")

IF( EXISTS "${Eigen3_DIR}/CMakeLists.txt" )
    MESSAGE( STATUS "Eigen3 found at ${Eigen3_DIR}")
ELSE()
	MESSAGE( FATAL_ERROR "Eigen3 not found")
ENDIF()

# CATCH2
if(DEFINED ENV{CATCH2_DIR})
    set(CATCH2_DIR $ENV{CATCH2_DIR} CACHE PATH  "Path to Catch2")
endif()

find_package(Catch2 3 REQUIRED PATHS ${CATCH2_DIR})

IF( ${Catch2_FOUND} )
    MESSAGE( STATUS "Catch2 found at ${Catch2_DIR}")
ENDIF()

# JSON
if(DEFINED ENV{nlohmann_json_DIR})
    set(nlohmann_json_DIR $ENV{nlohmann_json_DIR} CACHE PATH  "Path to nlohmann_json")
endif()

find_package(nlohmann_json 3.2.0 REQUIRED PATHS ${nlohmann_json_DIR})

IF( ${NLOHMANN_JSON_FOUND} )
    MESSAGE( STATUS "NLOHMANN_JSON found at ${nlohmann_json_DIR}")
ENDIF()

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(SOURCES
    test.cpp
	test_utils.h
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

# Include eigen headers 
target_include_directories(${PCA_TESTS} PRIVATE ${Eigen3_DIR})

# -----------------------------------------------------------------------------
# Target link directories
# -----------------------------------------------------------------------------
# linking against catch will add the proper include path
target_link_libraries(${PCA_TESTS} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${PCA_TESTS} PRIVATE nlohmann_json::nlohmann_json)

if(USE_OPENMP AND OpenMP_CXX_FOUND)
	message(STATUS "Link to OpenMP")
	target_link_libraries(${PCA_TESTS} PRIVATE OpenMP::OpenMP_CXX)
endif()

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------

# Request C++17
target_compile_features(${PCA_TESTS} PRIVATE cxx_std_20)
target_compile_features(${PCA_TESTS} PRIVATE c_std_17)

if(MSVC)
    target_compile_options(${PCA_TESTS} PRIVATE /bigobj)	# for Eigen
endif(MSVC)

# Instruction sets
set(AXV_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:AVX,-DUSE_AVX>)
set(AXV2_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:AVX2,-DUSE_AVX2>)
set(SSE2_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:SSE2,-DUSE_SSE2>)

check_cxx_compiler_flag(${AXV_CompileOption} COMPILER_OPT_AVX_SUPPORTED)
check_cxx_compiler_flag(${AXV2_CompileOption} COMPILER_OPT_AVX2_SUPPORTED)

if(${COMPILER_OPT_AVX2_SUPPORTED})
	MESSAGE( STATUS "Use AXV2")
	target_compile_options(${PCA_TESTS} PRIVATE ${AXV2_CompileOption})
elseif(${COMPILER_OPT_AVX_SUPPORTED})
	MESSAGE( STATUS "Use AXV")
	target_compile_options(${PCA_TESTS} PRIVATE ${AXV_CompileOption})
else()
	MESSAGE( STATUS "Use SSE2")
	target_compile_options(${PCA_TESTS} PRIVATE ${SSE2_CompileOption})
endif()

# Warning levels
if(MSVC)
  target_compile_options(${PCA_TESTS} PRIVATE /W3)
else()
  target_compile_options(${PCA_TESTS} PRIVATE -Wall)
endif()
