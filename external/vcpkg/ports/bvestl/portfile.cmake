include(vcpkg_common_functions)

vcpkg_from_gitlab(
    GITLAB_URL https://gitlab.bvereborn.com
    OUT_SOURCE_PATH SOURCE_PATH
    REPO bve-reborn/bvestl
    REF v0.2.3
    SHA512 aefe57e042dba4aeb5edb84f1a4c762a64ee005116c4920d1f7dedaee506de255ca4205eec5d243020bf51fa7b09c67185853c9e9320ea188bce42ca6555650b
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
