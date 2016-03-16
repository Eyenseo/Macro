set(TEST_TARGET cad::Macro)

string(REPLACE "::" "_" THIS_GROUP ${TEST_TARGET})

set(TEST_GROUP test_${THIS_GROUP})
set(TEST_FOLDER "Test Cad-Macro")
set(CHECK_GROUP check-${THIS_GROUP})

set(
  TEST_TARGETS
    AST
    Tokenizer
    Parser
)

if(${BUILD_TESTING})
  #########################################
  # Setup
  #########################################
  string(TOUPPER ${TEST_GROUP} TEST_GROUP_UPPER)
  set(
    ${TEST_GROUP_UPPER}
      ON
    CACHE
    BOOL
    "If TRUE the tests for ${THIS_GROUP} will be configured"
  )

  if(${${TEST_GROUP_UPPER}})
    if(NOT TARGET check)
      add_custom_target(
        check
          ${CMAKE_COMMAND}
            -E env CTEST_OUTPUT_ON_FAILURE=1
          ${CMAKE_CTEST_COMMAND}
            -C $<CONFIG>
          WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
      )
    endif()
    add_custom_target(${TEST_GROUP})
    add_custom_target(
      ${CHECK_GROUP}
        ${CMAKE_COMMAND}
          -E env CTEST_OUTPUT_ON_FAILURE=1
        ${CMAKE_CTEST_COMMAND}
          -C $<CONFIG>
        WORKING_DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR}

    )
    add_dependencies(check ${TEST_GROUP})
    add_dependencies(${CHECK_GROUP} ${TEST_GROUP})
    set_property(TARGET ${TEST_GROUP} PROPERTY FOLDER ${TEST_FOLDER})
    set_property(TARGET ${CHECK_GROUP} PROPERTY FOLDER ${TEST_FOLDER})

    foreach(TEST ${TEST_TARGETS})
      set(TEST_NAME "${TEST_GROUP}_${TEST}")
      string(TOUPPER ${TEST_NAME} TEST_NAME_UPPER)

      set(
        ${TEST_NAME_UPPER}
          ON
        CACHE
        BOOL
        "If TRUE the tests for ${TEST_TARGET} ${TEST} will be configured"
      )

      if(${${TEST_NAME_UPPER}} OR ${TEST_GROUP_UPPER})
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${TEST}")
      endif()
    endforeach()
  endif()
endif()
