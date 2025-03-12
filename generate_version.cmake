# Git executable is extracted from parameters.
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always
    OUTPUT_VARIABLE GIT_REPO_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date +%Y-%m-%d
    OUTPUT_VARIABLE BUILD_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(GIT_REPO_VERSION STREQUAL "")
  execute_process(COMMAND kacl-cli -f ${CHANGELOG_FILE} current
    OUTPUT_VARIABLE KACL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(GIT_REPO_VERSION v${KACL_VERSION})
  message(WARNING "Setting version from CHANGELOG.md using kacl-cli.")
endif()

if(GIT_REPO_VERSION STREQUAL "v")
  set(GIT_REPO_VERSION v0.0.0-unknown)
  message(WARNING "Failed to determine version from Git tags or kacl-cli. Using default version \"${GIT_REPO_VERSION}\".")
endif()

# Input and output files are extracted from parameters.
configure_file(${INPUT_FILE} ${OUTPUT_FILE})

