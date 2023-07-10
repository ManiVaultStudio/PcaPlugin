cmake_minimum_required(VERSION 3.21)

option(USE_OPENMP "Use OpenMP - by default ON" ON)
option(USE_AVX "Use AVX if available - by default ON" OFF)

# -----------------------------------------------------------------------------
# Project: PCA plugin
# -----------------------------------------------------------------------------
set(PCA_PLUGIN "PcaPlugin")

PROJECT(${PCA_PLUGIN} C CXX)

# -----------------------------------------------------------------------------
# Set cmake flags
# -----------------------------------------------------------------------------

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# Test hardware avx capabilities
include(CMakeCheckSetAVX)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
if (NOT DEFINED ENV{HDPS_INSTALL_DIR})
	message(FATAL_ERROR "Environment variable HDPS_INSTALL_DIR not set.")
endif()

file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)
set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "..." FORCE)

message(STATUS "Install plugin to ${INSTALL_DIR}")

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(OpenMP)

set(Eigen3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen")

IF(EXISTS "${Eigen3_DIR}/CMakeLists.txt")
    MESSAGE(STATUS "Eigen3 found at ${Eigen3_DIR}")
ELSE()
	MESSAGE(FATAL_ERROR "Eigen3 not found")
ENDIF()

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(PCA_PLUGIN_SOURCES
    src/PcaPlugin.h
    src/PcaPlugin.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/DimensionSelectionAction.h
    src/DimensionSelectionAction.cpp
)

set(PLUGIN_MOC_HEADERS
    src/PcaPlugin.h
)

set(PCA_HEADERS
    src/PCA.h
)

set(AUX
    src/PcaPlugin.json
)


source_group(Plugin FILES ${SPH_PLUGIN_SOURCES})
source_group(PCA FILES ${SPH_PLUGIN_SOURCES})
source_group(Aux FILES ${AUX})


# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------

add_library(${PCA_PLUGIN} SHARED ${PCA_PLUGIN_SOURCES} ${PCA_HEADERS} ${AUX})

qt_wrap_cpp(PCA_MOC ${PLUGIN_MOC_HEADERS} TARGET ${PCA_PLUGIN})
target_sources(${PCA_PLUGIN} PRIVATE ${PCA_MOC})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------

# Include HDPS core headers
target_include_directories(${PCA_PLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PCA_PLUGIN} PRIVATE ${Eigen3_DIR})

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# link Qt
target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::WebEngineWidgets)

# link ManiVault core and data plugins
set(HDPS_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 

target_link_libraries(${PCA_PLUGIN} PRIVATE "${HDPS_LINK_LIBRARY}")
target_link_libraries(${PCA_PLUGIN} PRIVATE "${POINTDATA_LINK_LIBRARY}")

if(USE_OPENMP AND OpenMP_CXX_FOUND)
	message(STATUS "Link to OpenMP")
	target_link_libraries(${PCA_PLUGIN} PRIVATE OpenMP::OpenMP_CXX)
endif()

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------

# Request C++17
target_compile_features(${PCA_PLUGIN} PRIVATE cxx_std_17)
target_compile_features(${PCA_PLUGIN} PRIVATE c_std_17)

if(MSVC)
    target_compile_options(${PCA_PLUGIN} PRIVATE /bigobj)	# for Eigen
endif(MSVC)

# Instruction sets
check_and_set_AVX(${PCA_PLUGIN} ${USE_AVX})

# Warning levels
if(MSVC)
  target_compile_options(${PCA_PLUGIN} PRIVATE /W3)
else()
  target_compile_options(${PCA_PLUGIN} PRIVATE -Wall)
endif()

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${PCA_PLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PCA_PLUGIN} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
if(MSVC)
	# Automatically set the debug environment (command + working directory) for MSVC 
	set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug,${INSTALL_DIR}/release>)
	set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug/HDPS.exe,${INSTALL_DIR}/release/HDPS.exe>) 
endif()

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
