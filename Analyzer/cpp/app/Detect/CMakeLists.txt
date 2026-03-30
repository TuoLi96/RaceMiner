add_executable(detect_bugs
    detect_bugs.cpp
)

target_include_directories(detect_bugs
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(detect_bugs
    PRIVATE
    AliasAnalysis
    ModMgr
	CFG
	ConcAPI
	RaceDetector
	PathMgr
	DBMgr
	Utils
	miner_flags
)

install(TARGETS detect_bugs
	RUNTIME DESTINATION bin
)
