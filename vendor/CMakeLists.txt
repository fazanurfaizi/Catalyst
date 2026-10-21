include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# ============================================================
# GLFW — Windows and Input
# ============================================================
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

# GLM — OpenGL Mathematics
set(GLM_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
  GIT_SHALLOW    TRUE
)

# STB Image — header-only, no CMakeLists.txt in upstream
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)

# GLAD — OpenGL loader (glad2 + CMake helper)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v2.0.6
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  cmake
)

# ImGui — Immediate Mode UI
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.5
  GIT_SHALLOW    TRUE
)

# Populate everything
FetchContent_MakeAvailable(glfw glm)

# stb has no CMakeLists.txt - populate-only + INTERFACE target
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
  FetchContent_Populate(stb)
endif()
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# glad2 - populate, add its cmake module path, then generate loader
FetchContent_MakeAvailable(glad)
glad_add_library(glad REPRODUCIBLE API gl:core=3.3)

# ImGui - recent versions ship a CMakeLists.txt that builds `imgui`
FetchContent_MakeAvailable(imgui)

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

# Add the backends the game needs (ImGui's CMake does not build these by default)
target_sources(imgui PRIVATE
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw glad)
