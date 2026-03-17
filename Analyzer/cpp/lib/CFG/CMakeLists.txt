add_library(CFG
    CFG.cpp
	IntraCFG.cpp
)

target_include_directories(CFG
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
