cmake_minimum_required(VERSION 3.30)

project(pierre)

add_executable( pierre )

target_compile_features(pierre PUBLIC
	cxx_std_17
)

target_sources( pierre PRIVATE 
  source/main.cpp
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(Vulkan REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/json EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/simplex_noise)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/glm EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/SDL3_image EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/sockpp EXCLUDE_FROM_ALL)

# Link to the actual SDL3 library.
target_link_libraries(pierre PRIVATE
  simplex_noise
  glm
  nlohmann_json::nlohmann_json
  SDL3::SDL3
  SDL3_image::SDL3_image
  sockpp
  ${Vulkan_LIBRARIES}
)

target_include_directories(pierre PRIVATE
  ${CMAKE_SOURCE_DIR}/source
  ${Vulkan_INCLUDE_DIRS}
)