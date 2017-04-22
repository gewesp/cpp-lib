cmake_minimum_required(VERSION 3.0)

project(ccp-lib VERSION 1.0.0)

# use own cmake modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# 3rd-party.
find_package(Boost 1.58.0 REQUIRED)
find_package(png++ REQUIRED)
find_package(PkgConfig)
pkg_search_module(Eigen3 REQUIRED eigen3)

set(include_platform_name "")
if (UNIX)
    set(include_platform_name "posix")
endif()

# create symlink to allow compiling.
add_custom_target(link_platform
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_CURRENT_LIST_DIR}/include/cpp-lib/${include_platform_name}"
    "${CMAKE_CURRENT_LIST_DIR}/include/cpp-lib/platform")

set(public-headers
    "include/cpp-lib/array_ops.h"
    "include/cpp-lib/assert.h"
    "include/cpp-lib/audio.h"
    "include/cpp-lib/bg-typedefs.h"
    "include/cpp-lib/blowfish.h"
    "include/cpp-lib/cgi.h"
    "include/cpp-lib/command_line.h"
    "include/cpp-lib/container-util.h"
    "include/cpp-lib/crypt_stream.h"
    "include/cpp-lib/dispatch.h"
    "include/cpp-lib/exception.h"
    "include/cpp-lib/geodb.h"
    "include/cpp-lib/geometry.h"
    "include/cpp-lib/gnss.h"
    "include/cpp-lib/http.h"
    "include/cpp-lib/igc.h"
    "include/cpp-lib/interpolation.h"
    "include/cpp-lib/map.h"
    "include/cpp-lib/math-util.h"
    "include/cpp-lib/matrix-wrapper.h"
    "include/cpp-lib/minimize.h"
    "include/cpp-lib/nmea.h"
    "include/cpp-lib/ogn.h"
    "include/cpp-lib/optimization.h"
    "include/cpp-lib/quaternion.h"
    "include/cpp-lib/random.h"
    "include/cpp-lib/registry.h"
    "include/cpp-lib/registry-crypt.h"
    "include/cpp-lib/serial.h"
    "include/cpp-lib/spatial-index.h"
    "include/cpp-lib/top-n.h"
    "include/cpp-lib/units.h"
    "include/cpp-lib/varlist.h"
    "include/cpp-lib/xdr.h"
    )
set(private-headers
    "")
set(sources
    "src/audio.cpp"
    "src/blowfish.cpp"
    "src/cgi.cpp"
    "src/command_line.cpp"
    "src/dispatch.cpp"
    "src/geodb.cpp"
    "src/geometry.cpp"
    "src/gnss.cpp"
    "src/http.cpp"
    "src/igc.cpp"
    "src/interpolation.cpp"
    "src/map.cpp"
    "src/math-util.cpp"
    "src/nmea.cpp"
    "src/ogn.cpp"
    "src/random.cpp"
    "src/util.cpp"
    "src/varlist.cpp"
    )

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${Eigen3_INCLUDE_DIRS}
    "${png++_INCLUDE_DIRS}/png++"
    "include"
    )

add_library(cpp-lib
    ${public-headers}
    ${private-headers}
    ${sources}
    )
add_dependencies(cpp-lib link_platform)

# ensures cpp std version through compiler feature.
set_property(TARGET cpp-lib PROPERTY CXX_STANDARD 14)

# for library export definitions.
include(GenerateExportHeader)
generate_export_header(cpp-lib)

# install section.

# variables used in the install section.
set(config_install_dir "lib/cmake")
set(include_install_dir "include")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")

# not using namespaces yet.
set(namespace "${PROJECT_NAME}::")

# include tools for cmake packagins.
include(CMakePackageConfigHelpers)

# configure project versionning.
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# configure project to be included using cmake.
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# export targets
install(
    TARGETS cpp-lib
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
)

# install headers.
install(
    DIRECTORY "include/cpp-lib"
    DESTINATION "${include_install_dir}"
    FILES_MATCHING PATTERN "*.h"
    )

# library export.
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/cpp-lib_export.h"
    DESTINATION "${include_install_dir}"
    )

# config and version files.
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
    )

# target files.
install(
    EXPORT "${targets_export_name}"
    DESTINATION "${config_install_dir}"
    )
