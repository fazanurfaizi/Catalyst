include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GLFW — Windows and Input
set(GLFW_INSTALL         OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS      OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

# GLM - OpenGL Mathematics
set(GLM_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

# STB Image - header-only, no CMakeLists.txt in upstream
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
  FetchContent_Populate(stb)
endif()
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# GLAD - OpenGL loader (glad2 + CMake helper)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v2.0.8
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_33 REPRODUCIBLE EXCLUDE_FROM_ALL API gl:core=3.3)
target_include_directories(glad_gl_core_33 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

add_library(glad ALIAS glad_gl_core_33)

# ImGui — Immediate Mode UI
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.5
  GIT_SHALLOW    TRUE
)

# ImGui - recent versions ship a CMakeLists.txt that builds `imgui`
FetchContent_MakeAvailable(imgui)

# Json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Miniaudio
FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG        0.11.22
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(miniaudio)

if(NOT TARGET imgui)
  # Fallback for older ImGui tags without CMakeLists.txt
  file(GLOB IMGUI_CORE_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
  )
  add_library(imgui STATIC ${IMGUI_CORE_SOURCES})
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

if(TARGET glad)
    get_target_property(_glad_inc glad INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "[vendor] glad target EXISTS")
    message(STATUS "[vendor] glad INTERFACE_INCLUDE_DIRECTORIES = ${_glad_inc}")
else()
    message(FATAL_ERROR "[vendor] glad target does NOT exist")
endif()

# Add the backends the game needs (ImGui's CMake does not build these by default)
target_sources(imgui PRIVATE
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw glad)
