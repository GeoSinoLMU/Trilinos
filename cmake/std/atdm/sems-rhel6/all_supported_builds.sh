# This script is sourced to return all of the supported builds

export ATDM_CONFIG_CTEST_S_BUILD_NAME_PREFIX=Trilinos-atdm-

export ATDM_CONFIG_ALL_SUPPORTED_BUILDS=(
  sems-rhel6-gnu-7.2.0-openmp-debug
  sems-rhel6-gnu-7.2.0-openmp-release
  sems-rhel6-gnu-7.2.0-openmp-release-debug
  sems-rhel6-gnu-7.2.0-openmp-complex-shared-release-debug
  sems-rhel6-intel-17.0.1-openmp-release
  )
