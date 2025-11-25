# Install script for directory: /mnt/d/Project/ns-3-ub/src/applications

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-applications.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-applications.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/src/applications/helper/bulk-send-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/helper/on-off-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/helper/packet-sink-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/helper/three-gpp-http-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/helper/udp-client-server-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/helper/udp-echo-helper.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/application-packet-probe.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/bulk-send-application.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/onoff-application.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/packet-loss-counter.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/packet-sink.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/seq-ts-echo-header.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/seq-ts-header.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/seq-ts-size-header.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/sink-application.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/source-application.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/three-gpp-http-client.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/three-gpp-http-header.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/three-gpp-http-server.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/three-gpp-http-variables.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/udp-client.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/udp-echo-client.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/udp-echo-server.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/udp-server.h"
    "/mnt/d/Project/ns-3-ub/src/applications/model/udp-trace-client.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/applications-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/applications/examples/cmake_install.cmake")

endif()

