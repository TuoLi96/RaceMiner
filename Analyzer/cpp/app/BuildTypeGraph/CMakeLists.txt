add_executable(BuildTypeGraph
    build_type_graph.cpp
)

target_include_directories(BuildTypeGraph
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(BuildTypeGraph
    PRIVATE
    AliasAnalysis
    ModMgr
)
