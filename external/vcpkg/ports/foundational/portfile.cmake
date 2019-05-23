include(vcpkg_common_functions)

vcpkg_check_linkage(ONLY_SHARED_LIBRARY)

if ((NOT (${TARGET_TRIPLET} MATCHES "x64")) OR (${TARGET_TRIPLET} MATCHES UWP))
	message(FATAL_ERROR "Foundational only supports x64 at this time.")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO NovaMods/foundational
    REF v0.2.2
    SHA512 6edde8cb803d0f619b244ecb2b587156f27b9365b0c7350a9f3a6775c6c26bd410d68a6e69d72cae911731664ce88a2d2cef540dae53882fd073b9bfdcd856bf
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
