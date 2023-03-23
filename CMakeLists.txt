cmake_minimum_required(VERSION 3.21)

option(USE_OPENMP "Use OpenMP - by default ON" ON)

# -----------------------------------------------------------------------------
# Project: PCA plugin
# -----------------------------------------------------------------------------
set(PCA_PLUGIN "PcaPlugin")

PROJECT(${PCA_PLUGIN} C CXX)

# -----------------------------------------------------------------------------
# Set cmake flags
# -----------------------------------------------------------------------------

# set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
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

source_group(Plugin FILES ${SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------

add_library(${PCA_PLUGIN} SHARED ${SOURCES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------

# Include HDPS core headers
target_include_directories(${PCA_PLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")
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
target_link_libraries(${PCA_PLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${PCA_PLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")

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
set(AXV_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:AVX,-DUSE_AVX>)
set(AXV2_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:AVX2,-DUSE_AVX2>)
set(SSE2_CompileOption $<IF:$<CXX_COMPILER_ID:MSVC>,/arch:SSE2,-DUSE_SSE2>)

check_cxx_compiler_flag(${AXV_CompileOption} COMPILER_OPT_AVX_SUPPORTED)
check_cxx_compiler_flag(${AXV2_CompileOption} COMPILER_OPT_AVX2_SUPPORTED)

if(${COMPILER_OPT_AVX2_SUPPORTED})
	MESSAGE( STATUS "Use AXV2")
	target_compile_options(${PCA_PLUGIN} PRIVATE ${AXV2_CompileOption})
elseif(${COMPILER_OPT_AVX_SUPPORTED})
	MESSAGE( STATUS "Use AXV")
	target_compile_options(${PCA_PLUGIN} PRIVATE ${AXV_CompileOption})
else()
	MESSAGE( STATUS "Use SSE2")
	target_compile_options(${PCA_PLUGIN} PRIVATE ${SSE2_CompileOption})
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
install(TARGETS ${PCA_PLUGIN}
   RUNTIME DESTINATION Plugins COMPONENT SHAREDLIB   
   LIBRARY DESTINATION Plugins COMPONENT LINKLIB
   ARCHIVE DESTINATION lib COMPONENT SHAREDLIB
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
