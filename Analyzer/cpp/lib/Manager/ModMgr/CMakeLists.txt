llvm_map_components_to_libnames(LLVM_LIBS
    Core
    IRReader
    Support
)

add_library(ModMgr
    ModMgr.cpp
	ModPack.cpp
)

target_link_libraries(ModMgr
    ${LLVM_LIBS}
	miner_flags
)

target_include_directories(ModMgr
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
