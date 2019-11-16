set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
file(TO_CMAKE_PATH "${REPO_ROOT}" REPO_ROOT)

if(EXISTS "${REPO_ROOT}/.git" AND NOT EXISTS "${REPO_ROOT}/.git/hooks/hooks-revision")
    file(READ "${REPO_ROOT}/tools/git-hooks/hooks-revision" GIT_HOOKS_REVISION)
    message(STATUS "Missing git hooks, installing revision ${GIT_HOOKS_REVISION}")
    file(MAKE_DIRECTORY "${REPO_ROOT}/.git/hooks")

    file(GLOB GIT_HOOK_FILES LIST_DIRECTORIES false "${REPO_ROOT}/tools/git-hooks/*")
    foreach(GIT_HOOK_FILE IN ITEMS ${GIT_HOOK_FILES})
        get_filename_component(GIT_HOOK_FILE_NAME "${GIT_HOOK_FILE}" NAME)
        configure_file("${GIT_HOOK_FILE}" "${REPO_ROOT}/.git/hooks/${GIT_HOOK_FILE_NAME}" COPYONLY)
    endforeach()
endif()
