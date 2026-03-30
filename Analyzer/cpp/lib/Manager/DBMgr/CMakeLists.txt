add_library(DBMgr
    DBMgr.cpp
    TBLMgr.cpp
    LockCollectionMgr.cpp
	LinkMgr.cpp
)

target_include_directories(DBMgr
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

find_package(SQLite3 REQUIRED)

target_link_libraries(DBMgr
    PUBLIC
        SQLite::SQLite3
		Utils
		miner_flags
)
