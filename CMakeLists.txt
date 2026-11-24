cmake_minimum_required(VERSION 3.22)

set(CATALYST_VERSION_MAJOR 1)
set(CATALYST_VERSION_MINOR 0)
set(CATALYST_VERSION_PATCH 0)
set(CATALYST_VERSION_STRING ${CATALYST_VERSION_MAJOR}.${CATALYST_VERSION_MINOR}.${CATALYST_VERSION_PATCH})

project(Catalyst
  VERSION ${CATALYST_VERSION_STRING}
  DESCRIPTION "Catalyst"
  LANGUAGES C CXX
)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(CATALYST_IS_TOP_LEVEL ON)
else()
  set(CATALYST_IS_TOP_LEVEL OFF)
endif()

# Configure variables   
set(CATALYST_EXE_NAME ${PROJECT_NAME} CACHE STRING "Catalyst base executable name")
set(CATALYST_EXE_TARGET_NAME  ${CATALYST_EXE_NAME})

# Render Backend selection
set(CATALYST_RHI_BACKEND "opengl" CACHE STRING "RHI backend: opengl | vulkan")
set_property(CACHE CATALYST_RHI_BACKEND PROPERTY STRINGS opengl vulkan)

option(CATALYST_ENABLE_DEBUG "Enable debug build" OFF)
option(CATALYST_PRODUCTION_BUILD "Make this a production build" OFF)
option(CATALYST_BUILD_TESTS "Build the ${PROJECT_NAME} test programs" ${CATALYST_IS_TOP_LEVEL})
option(CATALYST_ENABLE_WARNING "Enable warning messages." ${CATALYST_IS_TOP_LEVEL})

# Set required C/C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)


# Configurations
# Default to build type "Release" unless debug is enabled
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	if (CATALYST_ENABLE_DEBUG)
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
set(CATALYST_WARNING_FLAGS "")
if(CATALYST_ENABLE_WARNING)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    list(APPEND CATALYST_WARNING_FLAGS
      -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
  elseif(MSVC)
    list(APPEND CATALYST_WARNING_FLAGS /W4)
  endif()

  if(CATALYST_WARNINGS_AS_ERRORS)
    if(MSVC)
      list(APPEND CATALYST_WARNING_FLAGS /WX)
    else()
      list(APPEND CATALYST_WARNING_FLAGS -Werror)
    endif()
  endif()
endif()

if(CATALYST_PRODUCTION_BUILD)
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
if(CATALYST_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

# Build targets
set(CATALYST_SOURCE_FILES "game/src/main.cpp")

# Add executable
add_executable("${CATALYST_EXE_TARGET_NAME}" "${CATALYST_SOURCE_FILES}")

target_compile_definitions("${CATALYST_EXE_TARGET_NAME}" PRIVATE GLFW_INCLUDE_NONE=1)

# Warnings
if(CATALYST_WARNING_FLAGS)
  target_compile_options(${CATALYST_EXE_TARGET_NAME} PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

target_link_libraries("${CATALYST_EXE_TARGET_NAME}" PRIVATE
  engine
  game
  imgui
)

if(CATALYST_PRODUCTION_BUILD)
  add_compile_definitions(PRODUCTION_BUILD=1)
  add_compile_definitions(ASSETS_PATH="./assets/")
else()
  add_compile_definitions(PRODUCTION_BUILD=0)
  add_compile_definitions(ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
endif()

# Install rules (so `cmake --install` produces a shippable layout)
if(CATALYST_PRODUCTION_BUILD AND CATALYST_IS_TOP_LEVEL)
  install(TARGETS ${CATALYST_EXE_TARGET_NAME}
    RUNTIME DESTINATION "."
  )
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    DESTINATION "assets"
  )
endif()
