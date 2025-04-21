cmake_minimum_required(VERSION 3.30)

project( simplex_noise )

add_library( simplex_noise )

target_sources( simplex_noise PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/SimplexNoise.cpp
)

target_include_directories( simplex_noise PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
)