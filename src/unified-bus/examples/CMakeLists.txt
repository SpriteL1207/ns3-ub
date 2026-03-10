set(ub_quick_example_libraries
    ${libcore}
    ${libconfig-store}
    ${libpoint-to-point}
)

if(${ENABLE_MTP})
  list(APPEND ub_quick_example_libraries ${libmtp})
endif()

build_lib_example(
  NAME ub-quick-example
  SOURCE_FILES ub-quick-example.cc
  LIBRARIES_TO_LINK ${ub_quick_example_libraries}
)
set(ub_mpi_config_smoke_libraries
    ${libcore}
    ${libconfig-store}
    ${libpoint-to-point}
    ${libmpi}
)

if(${ENABLE_MTP})
  list(APPEND ub_mpi_config_smoke_libraries ${libmtp})
endif()

build_lib_example(
  NAME ub-mpi-config-smoke
  SOURCE_FILES ub-mpi-config-smoke.cc
  LIBRARIES_TO_LINK ${ub_mpi_config_smoke_libraries}
)

if(${ENABLE_MPI} AND ${ENABLE_MTP})
  build_lib_example(
    NAME ub-hybrid-smoke
    SOURCE_FILES ub-hybrid-smoke.cc
    LIBRARIES_TO_LINK
      ${libmpi}
      ${libmtp}
      ${libnetwork}
  )
endif()
