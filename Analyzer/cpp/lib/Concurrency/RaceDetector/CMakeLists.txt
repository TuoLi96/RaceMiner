llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
)

find_package(spdlog REQUIRED)

add_library(RaceDetector
    LockCollector.cpp
	RaceDetector.cpp
)

target_link_libraries(RaceDetector
    ${LLVM_LIBS}
	spdlog::spdlog
	ModMgr
	AliasAnalysis
	CFG
	Utils
	miner_flags
)

target_include_directories(RaceDetector
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
