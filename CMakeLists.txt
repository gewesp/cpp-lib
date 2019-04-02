cmake_minimum_required(VERSION 3.0)

# TODO: Real version
project(cpp-lib VERSION 1.0.0)

# use own cmake modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Global paths
set(TEST_SRC_DIR "testing")

# 3rd-party.
find_package(Boost 1.58.0 REQUIRED)
find_package(png++ REQUIRED)
find_package(Eigen3 REQUIRED)


message("-- Boost libraries version:  ${Boost_VERSION}"            )
message("-- Boost include directory:  ${Boost_INCLUDE_DIRS}"       )

message("-- Eigen library version:    ${EIGEN3_VERSION_STRING}"    )
message("-- Eigen include directory:  ${EIGEN3_INCLUDE_DIRS}"      )

message("-- png++ include directory:  ${png++_INCLUDE_DIRS}/png++" )



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
    "include/cpp-lib/util.h"
    "include/cpp-lib/varlist.h"
    "include/cpp-lib/xdr.h"

    "include/cpp-lib/detail/socket_lowlevel.h"

    "include/cpp-lib/platform/net_impl.h"
    "include/cpp-lib/platform/realtime.h"
    "include/cpp-lib/platform/wrappers.h"

    "include/cpp-lib/sys/file.h"
    "include/cpp-lib/sys/logger.h"
    "include/cpp-lib/sys/network.h"
    "include/cpp-lib/sys/realtime.h"
    "include/cpp-lib/sys/server.h"
    "include/cpp-lib/sys/syslogger.h"
    "include/cpp-lib/sys/util.h"
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
    "src/network.cpp"
    "src/nmea.cpp"
    "src/ogn.cpp"
    "src/random.cpp"
    "src/registry.cpp"
    "src/util.cpp"
    "src/varlist.cpp"

    "src/sys/file.cpp"
    "src/sys/logger.cpp"
    "src/sys/server.cpp"
    "src/sys/syslogger.cpp"
    "src/sys/util.cpp"
    )

if (UNIX AND NOT APPLE)
    list(APPEND sources
         "src/posix/realtime.cpp"
         "src/posix/wrappers.cpp"
         )
endif()

############################################################################
# TARGET CONFIGURATION
############################################################################
add_executable(ogn-test
  ${TEST_SRC_DIR}/ogn-test.cpp)

add_library(cpp-lib
    ${public-headers}
    ${private-headers}
    ${sources}
    )

target_link_libraries(ogn-test
  cpp-lib
  pthread)


target_include_directories(cpp-lib PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${Eigen3_INCLUDE_DIRS}
    ${png++_INCLUDE_DIRS}/png++
    include
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${png++_INCLUDE_DIRS}/png++
)

# C++ standard
# TODO: Define standard library (see def.compiler)
set_target_properties(cpp-lib ogn-test PROPERTIES CXX_STANDARD 14)

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
