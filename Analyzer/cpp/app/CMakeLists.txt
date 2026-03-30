set(CMAKE_INSTALL_PREFIX "$ENV{MINER_ROOT}/Analyzer" CACHE PATH "Install path prefix" FORCE)

add_subdirectory(BuildTypeGraph)
add_subdirectory(Detect)
