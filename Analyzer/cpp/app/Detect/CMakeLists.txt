file(GLOB TOOL_SOURCES *.cpp)

foreach(src ${TOOL_SOURCES})
    get_filename_component(name ${src} NAME_WE)

    add_executable(${name} ${src})

    target_include_directories(${name}
        PRIVATE ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(${name}
        PRIVATE
        AliasAnalysis
        ModMgr
        CFG
        ConcAPI
        RaceDetector
        PathMgr
        DBMgr
        Utils
        miner_flags
    )

    install(TARGETS ${name}
        RUNTIME DESTINATION bin
    )
endforeach()
