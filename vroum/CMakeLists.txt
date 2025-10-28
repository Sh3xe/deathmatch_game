cmake_minimum_required(VERSION 3.30)

project( vroum )

add_library( vroum SHARED )

# C++ Standard
target_compile_features(vroum PUBLIC
	cxx_std_17
)

# Source files
target_sources( vroum PRIVATE
  source/graphics/graphics_system.cpp
  source/graphics/graphics_system.hpp
  source/application.cpp
  source/application.hpp
  source/logger.cpp
  source/logger.hpp
  source/layer.hpp
)

# Do not build any example / tests
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Libraries
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/json EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/simplex_noise)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL3_image EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sockpp EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad EXCLUDE_FROM_ALL)

# Link libraries
target_link_libraries(vroum PUBLIC
  simplex_noise
  glm
  nlohmann_json::nlohmann_json
  SDL3::SDL3
  SDL3_image::SDL3_image
  sockpp
  glad
)

# Header files
target_include_directories(vroum PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)