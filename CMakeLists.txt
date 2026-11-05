cmake_minimum_required(VERSION 3.16)

set(GAME_VERSION_MAJOR 1)
set(GAME_VERSION_MINOR 0)
set(GAME_VERSION_PATCH 0)
set(GAME_VERSION_STRING ${GAME_VERSION_MAJOR}.${GAME_VERSION_MINOR}.${GAME_VERSION_PATCH})

project(KingdomRushFrontiers
  VERSION ${GAME_VERSION_STRING}
  DESCRIPTION "Catalyst"
  LANGUAGES C CXX
)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GAME_IS_TOP_LEVEL ON)
else()
  set(GAME_IS_TOP_LEVEL OFF)
endif()

# Configure variables   
set(GAME_EXE_NAME ${PROJECT_NAME} CACHE STRING "Catalyst base executable name")
set(GAME_EXE_TARGET_NAME  ${GAME_EXE_NAME})

option(GAME_ENABLE_DEBUG "Enable debug build" OFF)
option(GAME_PRODUCTION_BUILD "Make this a production build" OFF)
option(GAME_BUILD_TESTS "Build the ${PROJECT_NAME} test programs" ${GAME_IS_TOP_LEVEL})
option(GAME_ENABLE_WARNING "Enable warning messages." ${GAME_IS_TOP_LEVEL})

# Set required C/C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)


# Configurations
# Default to build type "Release" unless debug is enabled
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	if (GAME_ENABLE_DEBUG)
		set(CMAKE_BUILD_TYPE "Debug")
	else()
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	message(STATUS "Build type not set, defaulting to ${CMAKE_BUILD_TYPE}")
endif()

# Output directories for a build tree
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Add compiler warnings
set(GAME_WARNING_FLAGS "")
if(GAME_ENABLE_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    list(APPEND GAME_WARNING_FLAGS
      -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
  elseif(MSVC)
    list(APPEND GAME_WARNING_FLAGS /W4)
  endif()

  if(GAME_WARNINGS_AS_ERRORS)
    if(MSVC)
      list(APPEND GAME_WARNING_FLAGS /WX)
    else()
      list(APPEND GAME_WARNING_FLAGS -Werror)
    endif()
  endif()
endif()

if(GAME_PRODUCTION_BUILD)
  add_compile_definitions(PRODUCTION_BUILD=1)
  add_compile_definitions(ASSETS_PATH="./assets/")
else()
  add_compile_definitions(PRODUCTION_BUILD=0)
  add_compile_definitions(ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
endif()

# Add Subdirectories
# Build vendor libraries
add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(game)

# Build the test apps   
if(GAME_BUILD_TESTS)
  include(CTest)
  enable_testing()
  # add_subdirectory(tests)
endif()

# Build targets
set(GAME_SOURCE_FILES "game/src/main.cpp")

# Add executable
add_executable("${GAME_EXE_TARGET_NAME}" "${GAME_SOURCE_FILES}")

target_compile_definitions("${GAME_EXE_TARGET_NAME}" PRIVATE GLFW_INCLUDE_NONE=1)

# Warnings
if(GAME_WARNING_FLAGS)
  target_compile_options(${GAME_EXE_TARGET_NAME} PRIVATE ${GAME_WARNING_FLAGS})
endif()

target_link_libraries("${GAME_EXE_TARGET_NAME}" PRIVATE
  engine
  game
  imgui
)

if(GAME_PRODUCTION_BUILD)
  add_compile_definitions(PRODUCTION_BUILD=1)
  add_compile_definitions(ASSETS_PATH="./assets/")
else()
  add_compile_definitions(PRODUCTION_BUILD=0)
  add_compile_definitions(ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
endif()

# Install rules (so `cmake --install` produces a shippable layout)
if(GAME_PRODUCTION_BUILD AND GAME_IS_TOP_LEVEL)
  install(TARGETS ${GAME_EXE_TARGET_NAME}
    RUNTIME DESTINATION "."
  )
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    DESTINATION "assets"
  )
endif()
