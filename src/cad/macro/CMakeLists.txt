target_sources(
  ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/MacroInitializer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/IndentBuffer.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/IndentStream.cpp
)

add_subdirectory("ast")
add_subdirectory("parser")
