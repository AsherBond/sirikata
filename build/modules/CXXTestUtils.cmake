MACRO(ADD_CXXTEST_CPP_TARGET)
  PARSE_ARGUMENTS(CXXTEST "DEPENDS;OUTPUTDIR;LIBRARYDIR" "" ${ARGN})
  CAR(CXXTEST_NAME ${CXXTEST_DEFAULT_ARGS})
  CDR(CXXTEST_FILES ${CXXTEST_DEFAULT_ARGS})

  SET(CXXTEST_EXEC_TARGET ${CXXTEST_NAME})
  SET(CXXTEST_H_FILES)
  SET(CXXTEST_OPTIONS --include=sirikata/core/util/Standard.hh)

  SET(CXXTEST_CPP_FILES)

  IF(PYTHON_EXECUTABLE)
    SET(CXXTEST_COMPILER ${PYTHON_EXECUTABLE})
    SET(CXXTEST_GEN ${CXXTEST_LIBRARYDIR}/bin/cxxtestgen)
  ENDIF()
  IF (CXXTEST_COMPILER)
    # Generate one --part file for each test
    FOREACH(CXXTEST_FILE ${CXXTEST_FILES})
      # Compute an output cpp path based on the filename
      GET_FILENAME_COMPONENT(CXXTEST_GENERATED_FILE ${CXXTEST_FILE} NAME)
      SET(CXXTEST_GENERATED_FILE "${GENERATED_FILES_DIR}/${CXXTEST_GENERATED_FILE}.cpp")
      # Setup partial test suite
      ADD_CUSTOM_COMMAND(OUTPUT ${CXXTEST_GENERATED_FILE}
        COMMAND ${CXXTEST_COMPILER} ${CXXTEST_GEN} ${CXXTEST_OPTIONS} --part -o ${CXXTEST_GENERATED_FILE} ${CXXTEST_FILE}
        DEPENDS ${CXXTEST_FILE} ${CXXTEST_DEPENDS} ${CXXTEST_GEN}
        COMMENT "Building ${CXXTEST_FILE} -> ${CXXTEST_GENERATED_FILE}")
      # Add to full list of generated files for build
      SET(CXXTEST_CPP_FILES ${CXXTEST_CPP_FILES} ${CXXTEST_GENERATED_FILE})
    ENDFOREACH()

    # Generate on --root file for the entire test suite
    SET(CXXTEST_RUNNER_FILE "${GENERATED_FILES_DIR}/test.cpp")
    ADD_CUSTOM_COMMAND(OUTPUT ${CXXTEST_RUNNER_FILE}
                       COMMAND ${CXXTEST_COMPILER} ${CXXTEST_GEN} ${CXXTEST_OPTIONS} --root -o ${CXXTEST_RUNNER_FILE}
                       DEPENDS ${CXXTEST_DEPENDS} ${CXXTEST_GEN}
                       COMMENT "Building ${CXXTEST_RUNNER_FILE}")
    # Add to full list of generated files for build
    SET(CXXTEST_CPP_FILES ${CXXTEST_CPP_FILES} ${CXXTEST_RUNNER_FILE})
  ELSE()
    ADD_CUSTOM_COMMAND(OUTPUT "no-file"
                       COMMAND exit 1
                       COMMENT "Unable to build ${CXXTEST_CPP_FILES} because python was not found.")
  ENDIF()

  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CXXTEST_CPP_FILES}")

ENDMACRO(ADD_CXXTEST_CPP_TARGET)
