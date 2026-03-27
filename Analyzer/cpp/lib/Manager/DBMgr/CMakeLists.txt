add_library(DBMgr
    DBMgr.cpp
    LockCollectionMgr.cpp
    TBLMgr.cpp
)

target_include_directories(DBMgr
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

find_package(SQLite3 REQUIRED)

target_link_libraries(DBMgr
    PUBLIC
        SQLite::SQLite3
)
