llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
)

find_package(spdlog REQUIRED)

add_library(ConcAPI
    LockAPI.cpp
)

target_link_libraries(ConcAPI
    ${LLVM_LIBS}
	spdlog::spdlog
	ModMgr
)

target_include_directories(ConcAPI
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
