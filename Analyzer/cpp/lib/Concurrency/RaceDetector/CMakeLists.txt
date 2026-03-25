llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
)

find_package(spdlog REQUIRED)

add_library(RaceDetector
    LockCollector.cpp
)

target_link_libraries(AliasAnalysis
    ${LLVM_LIBS}
	spdlog::spdlog
	ModMgr
	Utils
)

target_include_directories(RaceDetector
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
