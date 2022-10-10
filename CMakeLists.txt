cmake_minimum_required(VERSION 3.17)
# -----------------------------------------------------------------------------
# Project: PCA plugin
# -----------------------------------------------------------------------------
set(PCA_PLUGIN "PcaPlugin")

PROJECT(${PCA_PLUGIN} C CXX)

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

# use cmake hardware checks
include(CheckCXXCompilerFlag)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
if (NOT DEFINED ENV{HDPS_INSTALL_DIR})
	message(FATAL_ERROR "Environment variable HDPS_INSTALL_DIR not set.")
endif()

file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} HDPS_INSTALL_DIR)
set(CMAKE_INSTALL_PREFIX ${HDPS_INSTALL_DIR} CACHE PATH "..." FORCE)
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "Plugin install directory")

message(STATUS "Install plugin to ${INSTALL_DIR}")

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

# Qt6
find_package(Qt6 6.3.1 COMPONENTS Widgets WebEngineWidgets REQUIRED)

# OpenMP
find_package(OpenMP)

# EIGEN3
#if(DEFINED ENV{Eigen3_DIR})
#    set(Eigen3_DIR $ENV{Eigen3_DIR} CACHE PATH "Path to Eigen3")
#endif()

#find_package (Eigen3 3.4 REQUIRED NO_MODULE PATHS ${Eigen3_DIR})

set(Eigen3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen")

IF( EXISTS "${Eigen3_DIR}/CMakeLists.txt" )
    MESSAGE( STATUS "Eigen3 found at ${Eigen3_DIR}")
ELSE()
	MESSAGE( FATAL_ERROR "Eigen3 not found")
ENDIF()


# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(SOURCES
    src/PcaPlugin.h
    src/PcaPlugin.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/PcaPlugin.json
	src/PCA.h
	src/DimensionSelectionAction.h
	src/DimensionSelectionAction.cpp
)

source_group( Plugin FILES ${SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------

add_library(${PCA_PLUGIN} SHARED ${SOURCES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------

# Include HDPS core headers
target_include_directories(${PCA_PLUGIN} PRIVATE "${HDPS_INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PCA_PLUGIN} PRIVATE ${Eigen3_DIR})

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
if(MSVC)
    set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PCA_PLUGIN} PRIVATE "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${PCA_PLUGIN} PRIVATE "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")
target_link_libraries(${PCA_PLUGIN} PRIVATE OpenMP::OpenMP_CXX)
# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------

# Request C++17
target_compile_features(${PCA_PLUGIN} PRIVATE cxx_std_17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")	# strict standard conformance
    target_compile_options(${PCA_PLUGIN} PRIVATE /bigobj)	# for Eigen
endif(MSVC)

target_compile_definitions(${PCA_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

# Instruction sets
add_definitions(-DUSE_SSE)

if(MSVC)
	check_cxx_compiler_flag("/arch:AVX" COMPILER_OPT_AVX_SUPPORTED)
	if(${COMPILER_OPT_AVX_SUPPORTED})
		add_definitions(/arch:AVX)
	endif()
	
	check_cxx_compiler_flag("/arch:AVX2" COMPILER_OPT_AVX2_SUPPORTED)
	if(${COMPILER_OPT_AVX2_SUPPORTED})
		add_definitions(/arch:AVX2)
	endif()

else()
	check_cxx_compiler_flag("-DUSE_AVX" COMPILER_OPT_AVX_SUPPORTED)
	if(${COMPILER_OPT_AVX_SUPPORTED})
		add_definitions(-DUSE_AVX)
	endif()
	
	check_cxx_compiler_flag("-DUSE_AVX2" COMPILER_OPT_AVX2_SUPPORTED)
	if(${COMPILER_OPT_AVX2_SUPPORTED})
		add_definitions(-DUSE_AVX2)
	endif()

endif()

if(${COMPILER_OPT_AVX_SUPPORTED})
	MESSAGE( STATUS "Use AXV")
endif()
if(${COMPILER_OPT_AVX2_SUPPORTED})
	MESSAGE( STATUS "Use AXV2")
endif()

# Warning levels
if(MSVC)
  target_compile_options(${PCA_PLUGIN} PRIVATE /W3)
else()
  target_compile_options(${PCA_PLUGIN} PRIVATE -Wall)
endif()

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PCA_PLUGIN} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
	"$<TARGET_FILE:${PCA_PLUGIN}>"
	"$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${PCA_PLUGIN}>"
)

# Automatically set the debug environment (command + working directory) for MSVC in debug mode
set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug,${INSTALL_DIR}/release>)
set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug/HDPS.exe,${INSTALL_DIR}/release/HDPS.exe>) 

# -----------------------------------------------------------------------------
# Unit testing
# -----------------------------------------------------------------------------

if(NOT UNIT_TESTS)
    set(UNIT_TESTS FALSE CACHE BOOL "Create unit tests")
endif()

if(${UNIT_TESTS})
	MESSAGE( STATUS "Activate unit tests")
	add_subdirectory("test")
endif()
