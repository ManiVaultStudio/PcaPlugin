cmake_minimum_required(VERSION 3.17)
# -----------------------------------------------------------------------------
# Project: PCA plugin
# -----------------------------------------------------------------------------
set(PCA_PLUGIN "PcaPlugin")

PROJECT(${PCA_PLUGIN})

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

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

IF( NOT DEFINED EIGEN3_INCLUDE_DIR )
    MESSAGE( FATAL_ERROR "Define EIGEN3_INCLUDE_DIR, the directory of Eigen3")
ELSE()
	MESSAGE( STATUS "Eigen headers at ${EIGEN3_INCLUDE_DIR}")
ENDIF()
# don't use find_package (Eigen3 3.4 REQUIRED) for now since this requires more user setup

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
	src/Utils.h
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

# Include external headers
target_include_directories(${PCA_PLUGIN} PRIVATE "${EIGEN3_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------

target_link_libraries(${PCA_PLUGIN} Qt5::Widgets)
target_link_libraries(${PCA_PLUGIN} Qt5::WebEngineWidgets)
target_link_libraries(${PCA_PLUGIN} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/HDPS_Public.lib")
target_link_libraries(${PCA_PLUGIN} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/PointData.lib")

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

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PCA_PLUGIN} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
	"$<TARGET_FILE:${PCA_PLUGIN}>"
	"$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${PCA_PLUGIN}>"
)
