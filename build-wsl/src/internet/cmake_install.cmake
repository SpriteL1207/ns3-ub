# Install script for directory: /mnt/d/Project/ns-3-ub/src/internet

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-internet.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-internet.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/src/internet/helper/internet-stack-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/internet-trace-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-address-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-global-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-interface-container.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-list-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv4-static-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv6-address-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv6-interface-container.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv6-list-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv6-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ipv6-static-routing-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/neighbor-cache-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/rip-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/helper/ripng-helper.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/arp-cache.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/arp-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/arp-l3-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/arp-queue-disc-item.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/candidate-queue.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/global-route-manager-impl.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/global-route-manager.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/global-router-interface.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/icmpv4-l4-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/icmpv4.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/icmpv6-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/icmpv6-l4-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ip-l4-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-address-generator.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-end-point-demux.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-end-point.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-global-routing.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-interface-address.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-interface.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-l3-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-list-routing.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-packet-filter.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-packet-info-tag.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-packet-probe.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-queue-disc-item.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-raw-socket-factory.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-raw-socket-impl.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-route.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-routing-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-routing-table-entry.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4-static-routing.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv4.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-address-generator.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-end-point-demux.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-end-point.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-extension-demux.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-extension-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-extension.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-interface-address.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-interface.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-l3-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-list-routing.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-option-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-option.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-packet-filter.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-packet-info-tag.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-packet-probe.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-pmtu-cache.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-queue-disc-item.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-raw-socket-factory.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-route.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-routing-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-routing-table-entry.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6-static-routing.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ipv6.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/loopback-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ndisc-cache.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/rip-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/rip.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ripng-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/ripng.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/rtt-estimator.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-bbr.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-bic.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-congestion-ops.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-cubic.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-dctcp.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-highspeed.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-htcp.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-hybla.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-illinois.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-l4-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-ledbat.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-linux-reno.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-lp.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option-rfc793.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option-sack-permitted.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option-sack.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option-ts.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option-winscale.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-option.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-prr-recovery.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-rate-ops.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-recovery-ops.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-rx-buffer.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-scalable.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-socket-base.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-socket-factory.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-socket-state.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-socket.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-tx-buffer.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-tx-item.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-vegas.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-veno.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-westwood-plus.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/tcp-yeah.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/udp-header.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/udp-l4-protocol.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/udp-socket-factory.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/udp-socket.h"
    "/mnt/d/Project/ns-3-ub/src/internet/model/windowed-filter.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/internet-module.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/internet-export.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/internet/examples/cmake_install.cmake")

endif()

