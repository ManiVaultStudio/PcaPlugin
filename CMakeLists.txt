cmake_minimum_required(VERSION 3.21)

option(MV_PCA_USE_OPENMP "Use OpenMP - by default ON" ON)
option(MV_PCA_USE_AVX "Use AVX if available - by default ON" OFF)
option(MV_PCA_UNIT_TESTS "Create unit tests - by default OFF" OFF)
option(MV_UNITY_BUILD "Combine target source files into batches for faster compilation" OFF)

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
# Dependencies
# -----------------------------------------------------------------------------

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(OpenMP)

find_package(ManiVault COMPONENTS Core PointData CONFIG)

include(FetchContent)
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

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(PLUGIN_SOURCES
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

source_group(Plugin FILES ${PLUGIN_SOURCES})
source_group(PCA FILES ${PCA_HEADERS})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${PCA_PLUGIN} SHARED ${PLUGIN_SOURCES} ${PCA_HEADERS} ${AUX})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------

target_include_directories(${PCA_PLUGIN} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# link Qt
target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${PCA_PLUGIN} PRIVATE Qt6::WebEngineWidgets)

target_link_libraries(${PCA_PLUGIN} PRIVATE ManiVault::Core)
target_link_libraries(${PCA_PLUGIN} PRIVATE ManiVault::PointData)
target_link_libraries(${PCA_PLUGIN} PRIVATE Eigen3::Eigen)

if(${MV_PCA_USE_OPENMP} AND OpenMP_CXX_FOUND)
	message(STATUS "Link ${PCA_PLUGIN} to OpenMP")
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
check_and_set_AVX(${PCA_PLUGIN} ${MV_PCA_USE_AVX})

# Warning levels
if(MSVC)
  target_compile_options(${PCA_PLUGIN} PRIVATE /W3)
else()
  target_compile_options(${PCA_PLUGIN} PRIVATE -Wall)
endif()

# unity/jumbo build
if(MV_UNITY_BUILD)
    set_target_properties(${PCA_PLUGIN} PROPERTIES UNITY_BUILD ON)
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
	--prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
if(MSVC)
	# Automatically set the debug environment (command + working directory) for MSVC 
	set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
	set_property(TARGET ${PCA_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/debug/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/release/ManiVault Studio.exe">) 
endif()

# -----------------------------------------------------------------------------
# Unit testing
# -----------------------------------------------------------------------------
if(${MV_PCA_UNIT_TESTS})
	MESSAGE( STATUS "Activate unit tests")
	add_subdirectory("test")
endif()
