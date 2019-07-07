include(vcpkg_common_functions)

vcpkg_from_gitlab(
    GITLAB_URL https://gitlab.bvereborn.com
    OUT_SOURCE_PATH SOURCE_PATH
    REPO bve-reborn/bvestl
    REF v0.2.2
    SHA512 ba8b9649f5bbdd5fc874eb8e439160a05e145d3325a9cf3c1f4380704cbb0eed1b1360d666e57f2de3ce02895870b93d7bb501f4ad99260f468ac26e16a1ffd9
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/bvestl RENAME copyright)
