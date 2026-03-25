llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
)

find_package(spdlog REQUIRED)

add_library(AliasAnalysis
    TypeGraph.cpp
	AliasGraph.cpp
	Steensgaard.cpp
)

target_link_libraries(AliasAnalysis
    ${LLVM_LIBS}
	spdlog::spdlog
	ModMgr
	Utils
)

target_include_directories(AliasAnalysis
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
