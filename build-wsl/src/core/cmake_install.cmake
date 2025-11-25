# Install script for directory: /mnt/d/Project/ns-3-ub/src/core

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-core.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-core.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/build/include/ns3/core-config.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/int64x64-128.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/example-as-test.h"
    "/mnt/d/Project/ns-3-ub/src/core/helper/csv-reader.h"
    "/mnt/d/Project/ns-3-ub/src/core/helper/event-garbage-collector.h"
    "/mnt/d/Project/ns-3-ub/src/core/helper/random-variable-stream-helper.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/abort.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/ascii-file.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/ascii-test.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/assert.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/attribute-accessor-helper.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/attribute-construction-list.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/attribute-container.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/attribute-helper.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/attribute.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/boolean.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/breakpoint.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/build-profile.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/calendar-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/callback.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/command-line.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/config.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/default-deleter.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/default-simulator-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/demangle.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/deprecated.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/des-metrics.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/double.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/enum.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/event-id.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/event-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/fatal-error.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/fatal-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/fd-reader.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/environment-variable.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/global-value.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/hash-fnv.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/hash-function.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/hash-murmur3.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/hash.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/heap-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/int64x64-double.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/int64x64.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/integer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/length.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/list-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/log-macros-disabled.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/log-macros-enabled.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/log.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/make-event.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/map-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/math.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/names.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/node-printer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/nstime.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object-base.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object-factory.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object-map.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object-ptr-container.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object-vector.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/object.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/pair.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/pointer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/priority-queue-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/ptr.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/random-variable-stream.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/rng-seed-manager.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/rng-stream.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/show-progress.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/shuffle.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/simple-ref-count.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/simulation-singleton.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/simulator-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/simulator.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/singleton.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/string.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/synchronizer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/system-path.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/system-wall-clock-ms.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/system-wall-clock-timestamp.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/test.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/time-printer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/timer-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/timer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/trace-source-accessor.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/traced-callback.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/traced-value.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/trickle-timer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/tuple.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/type-id.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/type-name.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/type-traits.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/uinteger.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/uniform-random-bit-generator.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/valgrind.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/vector.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/warnings.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/watchdog.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/realtime-simulator-impl.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/wall-clock-synchronizer.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/val-array.h"
    "/mnt/d/Project/ns-3-ub/src/core/model/matrix-array.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/core-module.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/core/examples/cmake_install.cmake")

endif()

