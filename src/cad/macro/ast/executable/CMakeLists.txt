target_sources(
  ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/EntryFunction.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Executable.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Function.cpp
)
add_subdirectory(operator)
