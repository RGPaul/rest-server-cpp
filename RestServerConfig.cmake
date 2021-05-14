include (CMakeFindDependencyMacro)
find_dependency (Boost 1.76.0)
find_dependency (nlohmann_json)
include ("${CMAKE_CURRENT_LIST_DIR}/RestServerTargets.cmake")
