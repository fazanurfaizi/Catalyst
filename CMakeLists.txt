cmake_minimum_required(VERSION 3.16)

set(GAME_VERSION_MAJOR 1)
set(GAME_VERSION_MINOR 0)
set(GAME_VERSION_PATCH 0)
set(GAME_VERSION_STRING ${GAME_VERSION_MAJOR}.${GAME_VERSION_MINOR}.${GAME_VERSION_PATCH})

project(KingdomRushFrontiers
  VERSION ${GAME_VERSION_STRING}
  DESCRIPTION "Kingdom Rush Frontiers"
  LANGUAGES C CXX
)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GAME_IS_TOP_LEVEL ON)
else()
  set(GAME_IS_TOP_LEVEL OFF)
endif()

# Configure variables   
set(GAME_EXE_NAME ${PROJECT_NAME} CACHE STRING "Kingdom Rush Frontiers base executable name")

set(GAME_EXE_TARGET_NAME  ${GAME_EXE_NAME})

option(GAME_ENABLE_DEBUG "Enable debug build" OFF)
option(GAME_PRODUCTION_BUILD "Make this a production build" OFF)
option(GAME_BUILD_TESTS "Build the ${PROJECT_NAME} test programs" ${GAME_IS_TOP_LEVEL})
option(GAME_ENABLE_WARNING "Enable warning messages." ${GAME_IS_TOP_LEVEL})

# Set required C/C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENTIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


# Configurations
# Default to build type "Release" unless debug is enabled
if(NOT CMAKE_BUILD_TYPE)
	if (NOT GAME_ENABLE_DEBUG)
		message(STATUS "No build type selected, default to Release")
		set(CMAKE_BUILD_TYPE "Release")
	else()
		message(STATUS "No build type selected but tests are being built, default to Debug")
		set(CMAKE_BUILD_TYPE "Debug")
	endif()
endif()

# Add compiler warnings
if(GAME_ENABLE_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    set(COMPILER_WARNINGS -Werror -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
  endif()
endif()

# Output directories for a build tree
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Add Subdirectories

# Build vendor libraries
add_subdirectory(vendor)

# Build the test apps   
if(GAME_BUILD_TESTS)
  include(CTest)
  enable_testing()
  # add_subdirectory(tests)
endif()

# Build targets
include_directories("." "./include" "vendor/imgui" "vendor/glm" "vendor/glfw/include" "vendor/glad/include" "vendor/stb-image/include")

set(GAME_SOURCE_FILES "src/main.cpp")

# Add executable
add_executable("${GAME_EXE_TARGET_NAME}" "${GAME_SOURCE_FILES}")
target_compile_definitions("${GAME_EXE_TARGET_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)
target_link_libraries("${GAME_EXE_TARGET_NAME}" glm glfw glad imgui)

if(GAME_PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${GAME_EXE_TARGET_NAME}" PUBLIC ASSETS_PATH="./assets/") 
	target_compile_definitions("${GAME_EXE_TARGET_NAME}" PUBLIC PRODUCTION_BUILD=1) 
else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${GAME_EXE_TARGET_NAME}" PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
	target_compile_definitions("${GAME_EXE_TARGET_NAME}" PUBLIC PRODUCTION_BUILD=0) 
endif()
