include (CMakeFindDependencyMacro)
find_dependency (Boost 1.74.0)
find_dependency (NlohmannJson)
include ("${CMAKE_CURRENT_LIST_DIR}/RestServerTargets.cmake")
