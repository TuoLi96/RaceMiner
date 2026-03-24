add_library(Utils
    IROperations.cpp
)

target_include_directories(Utils
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
