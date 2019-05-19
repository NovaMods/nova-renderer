include(vcpkg_common_functions)

vcpkg_check_linkage(ONLY_SHARED_LIBRARY)

if ((NOT (${TARGET_TRIPLET} MATCHES "x64")) OR (${TARGET_TRIPLET} MATCHES UWP))
	message(FATAL_ERROR "Foundational only supports x64 at this time.")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO NovaMods/foundational
    REF v0.2.0
    SHA512 680c3c002bc2b8ae5be0bb21f5b77b6ec912be71f214d0806e3ab121bf234d56ba324836e661c0266b1ef42a1306c447954f14cc7bb4c40c967eb6836dbaad47
    HEAD_REF master
)

vcpkg_download_distfile(
    CMAKE_HELPERS_PATH
    URLS https://github.com/NovaMods/cmake-helpers/archive/6bd0266725d55ebbba5bf7ca2e3be819c9a6606d.zip
    FILENAME NovaMods-cmake-helpers-6bd0266725d55ebbba5bf7ca2e3be819c9a6606d.zip
    SHA512 0cd8a920f5984a22914896b48ae413c2f98f6f6e3e668b6086d60161505184cef4ddc321a8e4e68aea3705686f012eadc072b9e5dcf78a711c15683d21a235c7
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH CMAKE_HELPERS_SOURCE_PATH
    ARCHIVE "${CMAKE_HELPERS_PATH}"
    WORKING_DIRECTORY "${SOURCE_PATH}/external/cmake-helpers"
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DNOVA_PACKAGE=True
        "-DCMAKE_MODULE_PATH=${CMAKE_HELPERS_SOURCE_PATH}"
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/foundational RENAME copyright)

vcpkg_copy_pdbs()
