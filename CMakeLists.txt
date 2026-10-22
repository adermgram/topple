cmake_minimum_required(VERSION 3.15)
project(topple CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# raylib 5.0 and Box2D 2.4's CMakeLists predate CMake 3.5, which CMake 4 rejects
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS 5.0
)

# Box2D comes as a release archive: much smaller than cloning the repository
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  box2d
  URL https://github.com/erincatto/box2d/archive/refs/tags/v2.4.1.zip
)

FetchContent_MakeAvailable(raylib box2d)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(topple ${SOURCES})
target_include_directories(topple PRIVATE src)
target_link_libraries(topple PRIVATE raylib box2d)
if(MSVC)
  target_compile_options(topple PRIVATE /W4)
else()
  target_compile_options(topple PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# Treat library headers as system headers so -Wall -Wextra only reports our code
if(raylib_SOURCE_DIR)
  target_include_directories(topple SYSTEM PRIVATE ${raylib_SOURCE_DIR}/src)
endif()
if(box2d_SOURCE_DIR)
  target_include_directories(topple SYSTEM PRIVATE ${box2d_SOURCE_DIR}/include)
endif()

if(MINGW)
  target_link_options(topple PRIVATE -static)
endif()

# Copy resources next to the exe (the game finds them via GetApplicationDirectory)
add_custom_command(TARGET topple POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:topple>/resources)

add_custom_target(run COMMAND topple DEPENDS topple
                  WORKING_DIRECTORY $<TARGET_FILE_DIR:topple>)

# `cmake --install build --prefix <dir> --component game` gathers what a player needs. The
# component keeps raylib's and Box2D's own install rules (headers, libraries) out of it.
install(TARGETS topple RUNTIME DESTINATION . COMPONENT game)
install(DIRECTORY resources DESTINATION . COMPONENT game)
install(FILES README.md LICENSE DESTINATION . COMPONENT game)
if(raylib_SOURCE_DIR)
  install(FILES ${raylib_SOURCE_DIR}/LICENSE DESTINATION licenses RENAME LICENSE-raylib.txt COMPONENT game)
endif()
if(box2d_SOURCE_DIR)
  install(FILES ${box2d_SOURCE_DIR}/LICENSE DESTINATION licenses RENAME LICENSE-box2d.txt COMPONENT game)
endif()
