include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO BVE-Reborn/bvestl
    REF v0.1.0
    SHA512 044d4c1650753ac4c60d3b92cdde10c379ca63feded880b7baeeaa64f2ea63670d1fda78315e71d65a434bfb0e1f777f25d4f95c83104c5a1d55211baa901ec9
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/bvestl RENAME copyright)
