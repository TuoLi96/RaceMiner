add_library(Utils
	StrOperations.cpp
    IROperations.cpp
    DBOperations.cpp
)

target_include_directories(Utils
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Utils
	miner_flags
)
