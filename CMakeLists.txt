cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

#########################################
# Project
#########################################
project(
  Macro
    LANGUAGES
      CXX
)

#########################################################################
#                              Main Target                              #
#########################################################################
add_library(
  ${PROJECT_NAME}
    SHARED
      ${CMAKE_CURRENT_SOURCE_DIR}/src/module.cpp
)
add_library(cad::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_subdirectory("src")
file(GLOB_RECURSE ${PROJECT_NAME}_HEADER "include/*.h*")
target_sources(
  ${PROJECT_NAME}
    PRIVATE
      ${${PROJECT_NAME}_HEADER}
)
# include root directory for path correct includes
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

    $<TARGET_PROPERTY:cad::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:exception::Exception,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:p3::common::ABI,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:p3::common::ModuleSystem,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    exception::Exception
    p3::common::ModuleSystem
)
if(${MSVC})
  target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
      cad::Core
  )
endif()
target_compile_options(
  ${PROJECT_NAME}
    PRIVATE
      -DP3_ABI_cad_module
)
# This is used to determine the appropriate standard to use
# see http://www.cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html
target_compile_features(
  ${PROJECT_NAME}
  PUBLIC
    $<TARGET_PROPERTY:exception::Exception,INTERFACE_COMPILE_FEATURES>
    $<TARGET_PROPERTY:p3::common::ModuleSystem,INTERFACE_COMPILE_FEATURES>
)

if(${BUILD_TESTING})
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()
