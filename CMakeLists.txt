cmake_minimum_required(VERSION 3.10)

message("-- Cpp-lib cmake file")
message("-- Project source directory: ${PROJECT_SOURCE_DIR}")

# This is a C++17 project
# Argh.  Cmake 3.5 doesn't support 17 for this variable.  Currently
# done in the CXXFLAGS by consequence...
set (CMAKE_CXX_STANDARD 17)

project(cpp-lib VERSION 0.82.1)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Some of our own modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# PNG++ dependent stuff: Only Unix
if (UNIX)
  set(PNG_STUFF TRUE)
else()
  set(PNG_STUFF FALSE)
endif()

# Required 3rd party stuff
# png++ is found using stuff in CMAKE_MODULE_PATH
find_package(Boost 1.65.0 REQUIRED)

# Eigen3, png++ isn't cmake compliant yet on my system.  Argh.
# Please provide env variables.
if (APPLE)
  # Eigen3
  # NOT DEFINED etc. *must* be capitalized!!
  if (NOT DEFINED ENV{EIGEN3_INCLUDE_DIR})
    message (FATAL_ERROR "Please provide the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
  else()
    include_directories (SYSTEM "$ENV{EIGEN3_INCLUDE_DIR}")
  endif()
else()
  find_package(Eigen3 REQUIRED)
  message("-- Eigen library version:    ${EIGEN3_VERSION_STRING}"    )
  message("-- Eigen include directory:  ${EIGEN3_INCLUDE_DIRS}"      )
endif()

message("-- Boost libraries version:  ${Boost_VERSION}"            )
message("-- Boost include directory:  ${Boost_INCLUDE_DIRS}"       )

if (PNG_STUFF)
  if (NOT APPLE)
    find_package(png++ REQUIRED)
    message("-- png++ include directory:  ${png++_INCLUDE_DIRS}/png++" )
  else()
    if (NOT DEFINED ENV{PNGPP_INCLUDE_DIR})
      message (FATAL_ERROR "Please provide the environment variable PNGPP_INCLUDE_DIR to the include directory of your PNG++ installation.")
    else()
      include_directories (SYSTEM "$ENV{PNGPP_INCLUDE_DIR}")
    endif()
    
    execute_process(
      COMMAND libpng-config --libdir 
      OUTPUT_VARIABLE PNG_LIBRARY_DIR)
    message("-- PNG library directory: ${PNG_LIBRARY_DIR}")
    # link_directories is harmful!  If we
    # have link_directories(${PNG_LIBRARY_DIR}) here,
    # cmake will refuse to link with cpp-lib itself.  WTF.
    find_library(PNG_LIBRARY
      name png
      HINTS ${PNG_LIBRARY_DIR})
    message("-- PNG library: ${PNG_LIBRARY}")
  endif()
else()
  message("-- Not including stuff dependent on libpng")
endif()

# Test targets (see testing/)
set(TESTS 
    audio-test
    blowfish-test
    blowfish-util
    command_line-test
    crypt_stream-test
    dispatch-test
    error-test
    # file-test
    gnss-test
    interpolation-test
    # iowait
    logger-test
    map-test
    math-test
    matrix-wrapper-test
    # netutil
    ogn-minimal
    ogn-test
    optimization-test
    # png-test
    quaternion-test
    random-test
    registry-test
    rosenbrock-search
    rtree-test
    # sched-test
    sched-thread-test
    spatial-index-test
    syslogger-test
    table-test
    tcp-test
    top-n-test
    udp-test
    utc-test
    util-test
    varlist-test)

# The serial library is only implemented using a Windows API
# TODO: Append that to cpp-lib sources as well
if (WINDOWS)
  list(APPEND TESTS serial-test)
endif()

if (PNG_STUFF)
  list(APPEND TESTS png-test)
endif()

############################################################################

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
    "include/cpp-lib/database.h"
    "include/cpp-lib/dispatch.h"
    "include/cpp-lib/error.h"
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
    "include/cpp-lib/memory.h"
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
    "src/database.cpp"
    "src/dispatch.cpp"
    "src/error.cpp"
    "src/geodb.cpp"
    "src/geometry.cpp"
    "src/gnss.cpp"
    "src/http.cpp"
    "src/igc.cpp"
    "src/interpolation.cpp"
    "src/map.cpp"
    "src/math-util.cpp"
    "src/memory.cpp"
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

if (UNIX)
  list(APPEND sources
    "src/posix/realtime.cpp"
    "src/posix/wrappers.cpp"
  )
endif()


############################################################################
# COMPILER CONFIGURATION
############################################################################
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DBOOST_DISABLE_ASSERTS")

############################################################################
# TARGET CONFIGURATION
############################################################################

add_library(cpp-lib
    ${public-headers}
    ${private-headers}
    ${sources}
    )

foreach(TEST ${TESTS})
  add_executable(${TEST} testing/${TEST}.cpp)
  target_link_libraries(${TEST} cpp-lib pthread)
  if (UNIX AND NOT APPLE)
    target_link_libraries(${TEST} rt)
  endif()
endforeach()

if(PNG_STUFF)
  if (APPLE)
    target_link_libraries(png-test ${PNG_LIBRARY})
  else()
    target_link_libraries(png-test png)
  endif()
endif()

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
  ${png++_INCLUDE_DIRS}/png++
)

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
