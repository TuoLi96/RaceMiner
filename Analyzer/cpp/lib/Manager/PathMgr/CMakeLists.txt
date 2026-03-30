add_library(PathMgr
    PathMgr.cpp
)

target_include_directories(PathMgr
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(PathMgr
	miner_flags
)
