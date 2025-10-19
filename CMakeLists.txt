cmake_minimum_required(VERSION 3.30)

project( eco_plus_dm )

add_executable( eco_plus_dm )

# C++ Standard
target_compile_features( eco_plus_dm PUBLIC cxx_std_17 )

# Source files
target_sources( eco_plus_dm PRIVATE
  source/main.cpp
  source/main_menu.cpp
)

add_subdirectory( vroum )

# Link libraries
target_link_libraries( eco_plus_dm PRIVATE vroum )

# Header files
target_include_directories(eco_plus_dm PRIVATE
  ${CMAKE_SOURCE_DIR}/source
)