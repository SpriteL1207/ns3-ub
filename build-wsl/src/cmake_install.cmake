# Install script for directory: /mnt/d/Project/ns-3-ub/src

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/antenna/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/aodv/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/applications/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/bridge/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/brite/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/buildings/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/click/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/config-store/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/core/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/csma/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/csma-layout/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/dsdv/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/dsr/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/energy/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/fd-net-device/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/flow-monitor/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/internet/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/internet-apps/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/lr-wpan/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/lte/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/mesh/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/mobility/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/netanim/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/network/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/nix-vector-routing/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/olsr/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/openflow/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/point-to-point/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/point-to-point-layout/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/propagation/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/sixlowpan/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/spectrum/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/stats/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/tap-bridge/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/topology-read/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/traffic-control/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/uan/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/unified-bus/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/virtual-net-device/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/wifi/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/wimax/cmake_install.cmake")
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/zigbee/cmake_install.cmake")

endif()

