# Install script for directory: /Users/spriteliu/ns-3-ub/src/unified-bus

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
    set(CMAKE_INSTALL_CONFIG_NAME "default")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/spriteliu/ns-3-ub/build/lib/libns3.44-unified-bus-default.dylib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-unified-bus-default.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-unified-bus-default.dylib")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/spriteliu/ns-3-ub/build/lib"
      -add_rpath "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-unified-bus-default.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-unified-bus-default.dylib")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-traffic-gen.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-app.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-controller.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-datalink.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-datatype.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-header.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-link.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-port.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-sliding-bitmap-window.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-switch.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-transaction.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-function.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-transport.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-routing-process.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-switch-allocator.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-utils.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-network-address.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-tp-connection-manager.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-ldst-api.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-ldst-thread.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-ldst-instance.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-congestion-control.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-caqm.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/protocol/ub-flow-control.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-queue-manager.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-tag.h"
    "/Users/spriteliu/ns-3-ub/src/unified-bus/model/ub-fault.h"
    "/Users/spriteliu/ns-3-ub/build/include/ns3/unified-bus-module.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/spriteliu/ns-3-ub/cmake-cache/src/unified-bus/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
