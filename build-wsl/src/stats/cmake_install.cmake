# Install script for directory: /mnt/d/Project/ns-3-ub/src/stats

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-stats.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-stats.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES "/mnt/d/Project/ns-3-ub/src/stats/model/sqlite-output.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/src/stats/model/sqlite-data-output.h"
    "/mnt/d/Project/ns-3-ub/src/stats/helper/file-helper.h"
    "/mnt/d/Project/ns-3-ub/src/stats/helper/gnuplot-helper.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/average.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/basic-data-calculators.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/boolean-probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/data-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/data-collection-object.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/data-collector.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/data-output-interface.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/double-probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/file-aggregator.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/get-wildcard-matches.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/gnuplot-aggregator.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/gnuplot.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/histogram.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/omnet-data-output.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/stats.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/time-data-calculators.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/time-probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/time-series-adaptor.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/uinteger-16-probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/uinteger-32-probe.h"
    "/mnt/d/Project/ns-3-ub/src/stats/model/uinteger-8-probe.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/stats-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/stats/examples/cmake_install.cmake")

endif()

