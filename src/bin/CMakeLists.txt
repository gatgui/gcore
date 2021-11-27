
add_executable(readbom readbom.cpp)
add_executable(writebom writebom.cpp)

set_target_properties(readbom writebom PROPERTIES
    VISIBILITY_INLINES_HIDDEN ON
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden")

if(LINK_STATIC_GCORE)
    target_link_libraries(readbom PRIVATE gcore_static)
    target_link_libraries(writebom PRIVATE gcore_static)
else()
    target_link_libraries(readbom PRIVATE gcore_shared)
    target_link_libraries(writebom PRIVATE gcore_shared)

    set(_rpath "$<IF:${APPLE},@executable_path,$ORIGIN>/../lib")

    set_target_properties(readbom writebom PROPERTIES
        BUILD_RPATH ${_rpath}
        INSTALL_RPATH ${_rpath})
endif()

install(TARGETS readbom writebom DESTINATION "bin")
