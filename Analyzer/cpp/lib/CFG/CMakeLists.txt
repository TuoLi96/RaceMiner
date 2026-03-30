add_library(CFG
    CFG.cpp
	IntraCFG.cpp
	IntraAcycleCFG.cpp
)

target_include_directories(CFG
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(CFG
	miner_flags
)
