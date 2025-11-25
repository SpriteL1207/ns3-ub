# Install script for directory: /mnt/d/Project/ns-3-ub/src/lte

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so"
         RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/mnt/d/Project/ns-3-ub/build/lib/libns3.44-lte.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so"
         OLD_RPATH "/mnt/d/Project/ns-3-ub/build/lib:::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:$ORIGIN/:$ORIGIN/../lib:/usr/local/lib64:$ORIGIN/:$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.44-lte.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/mnt/d/Project/ns-3-ub/src/lte/helper/emu-epc-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/cc-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/epc-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/lte-global-pathloss-database.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/lte-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/lte-hex-grid-enb-topology-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/lte-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/mac-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/no-backhaul-epc-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/phy-rx-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/phy-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/phy-tx-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/point-to-point-epc-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/radio-bearer-stats-calculator.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/radio-bearer-stats-connector.h"
    "/mnt/d/Project/ns-3-ub/src/lte/helper/radio-environment-map-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/a2-a4-rsrq-handover-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/a3-rsrp-handover-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/component-carrier-enb.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/component-carrier-ue.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/component-carrier.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/cqa-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-enb-application.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-enb-s1-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-gtpc-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-gtpu-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-mme-application.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-pgw-application.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-s11-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-s1ap-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-sgw-application.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-tft-classifier.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-tft.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-ue-nas.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-x2-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-x2-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/epc-x2.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/eps-bearer-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/eps-bearer.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/fdbet-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/fdmt-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/fdtbfq-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/ff-mac-common.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/ff-mac-csched-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/ff-mac-sched-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-amc.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-anr-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-anr.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-as-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-asn1-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ccm-mac-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ccm-rrc-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-chunk-processor.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-common.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-control-messages.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-cmac-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-component-carrier-manager.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-cphy-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-mac.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-phy-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-enb-rrc.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-distributed-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-enhanced-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-rrc-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ffr-soft-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-fr-hard-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-fr-no-op-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-fr-soft-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-fr-strict-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-handover-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-handover-management-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-harq-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-interference.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-mac-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-mi-error-model.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-pdcp-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-pdcp-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-pdcp-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-pdcp.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-phy-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-radio-bearer-info.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-radio-bearer-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-am-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-am.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-sdu-status-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-sequence-number.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-tag.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-tm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc-um.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rlc.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rrc-header.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rrc-protocol-ideal.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rrc-protocol-real.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-rrc-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-spectrum-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-spectrum-signal-parameters.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-spectrum-value-helper.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-ccm-rrc-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-cmac-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-component-carrier-manager.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-cphy-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-mac.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-net-device.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-phy-sap.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-power-control.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-ue-rrc.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/lte-vendor-specific-parameters.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/no-op-component-carrier-manager.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/no-op-handover-algorithm.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/pf-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/pss-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/rem-spectrum-phy.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/rr-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/simple-ue-component-carrier-manager.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/tdbet-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/tdmt-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/tdtbfq-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/src/lte/model/tta-ff-mac-scheduler.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/lte-module.h"
    "/mnt/d/Project/ns-3-ub/build/include/ns3/lte-export.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/d/Project/ns-3-ub/build-wsl/src/lte/examples/cmake_install.cmake")

endif()

