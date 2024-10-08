cmake_minimum_required(VERSION 3.21)

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

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Catch2)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(Catch2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

if(NOT ${Eigen3_FOUND})
    set(BUILD_TESTING OFF CACHE BOOL "Enable testing for Eigen" FORCE)
    set(EIGEN_BUILD_DOC OFF CACHE BOOL "Enable creation of Eigen documentation" FORCE)
    FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen
        GIT_TAG 9df21dc8b4b576a7aa5c0094daa8d7e8b8be60f0
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(Eigen3)
    find_package(Eigen3 CONFIG REQUIRED)
endif()

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
set(PCA_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${PCA_TESTS} PRIVATE "${PCA_PLUGIN_DIR}/src")

# Include eigen headers 
target_include_directories(${PCA_TESTS} PRIVATE ${Eigen3_DIR})

# -----------------------------------------------------------------------------
# Target link directories
# -----------------------------------------------------------------------------
# linking against catch will add the proper include path
target_link_libraries(${PCA_TESTS} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${PCA_TESTS} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PCA_TESTS} PRIVATE Eigen3::Eigen)

if(${MV_PCA_USE_OPENMP} AND OpenMP_CXX_FOUND)
	message(STATUS "Link ${PCA_TESTS} to OpenMP")
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
