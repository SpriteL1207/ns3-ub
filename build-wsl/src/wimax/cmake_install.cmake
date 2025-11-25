# Install script for directory: /mnt/d/Project/ns-3-ub/src/wimax

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-wimax.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-wimax.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/src/wimax/helper/wimax-helper.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-channel.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ss-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/cid.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/cid-factory.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ofdm-downlink-frame-prefix.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-connection.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ss-record.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/mac-messages.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/dl-mac-messages.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ul-mac-messages.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-phy.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/simple-ofdm-wimax-phy.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/simple-ofdm-wimax-channel.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/send-params.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/service-flow.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ss-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/connection-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-mac-header.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-mac-queue.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/crc8.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/service-flow-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-uplink-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-uplink-scheduler-simple.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-uplink-scheduler-mbqos.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-uplink-scheduler-rtps.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ul-job.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-scheduler-simple.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-scheduler-rtps.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/service-flow-record.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/snr-to-block-error-rate-record.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/snr-to-block-error-rate-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/simple-ofdm-send-param.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ss-service-flow-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bs-service-flow-manager.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/cs-parameters.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ipcs-classifier-record.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-tlv.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/ipcs-classifier.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/bvec.h"
    "/mnt/d/Project/ns-3-ub/src/wimax/model/wimax-mac-to-mac-header.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/wimax-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/wimax/examples/cmake_install.cmake")

endif()

