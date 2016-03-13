target_sources(
  ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/AST.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Scope.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Define.cpp
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/executable)
