vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO sewenew/redis-plus-plus
    REF df522812ba4114f1dd3386b81afc2369c82b717d # 1.2.3
    SHA512 2ac59c5416ba85a60061d941787cb3bc2e43042ca0a53829f866448015ed6800c38ecbde3319d9756a70bfba2860732136d89d296382a6b9a675bbe32173f22b
    HEAD_REF master
    PATCHES
        fix-conversion.patch
        find-args.patch
        require-hiredis.patch
)

if("cxx17" IN_LIST FEATURES)
    set(REDIS_PLUS_PLUS_CXX_STANDARD 17)
else()
    set(REDIS_PLUS_PLUS_CXX_STANDARD 11)
endif()

if (VCPKG_LIBRARY_LINKAGE STREQUAL static)
    set(REDIS_PLUS_PLUS_BUILD_STATIC ON)
    set(REDIS_PLUS_PLUS_BUILD_SHARED OFF)
else()
    set(REDIS_PLUS_PLUS_BUILD_STATIC OFF)
    set(REDIS_PLUS_PLUS_BUILD_SHARED ON)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DREDIS_PLUS_PLUS_USE_TLS=OFF
        -DREDIS_PLUS_PLUS_BUILD_STATIC=${REDIS_PLUS_PLUS_BUILD_STATIC}
        -DREDIS_PLUS_PLUS_BUILD_SHARED=${REDIS_PLUS_PLUS_BUILD_SHARED}
        -DREDIS_PLUS_PLUS_BUILD_TEST=OFF
        -DREDIS_PLUS_PLUS_CXX_STANDARD=${REDIS_PLUS_PLUS_CXX_STANDARD}
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH share/cmake/redis++ TARGET_PATH share/redis++)

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright )
