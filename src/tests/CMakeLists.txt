
file(GLOB prog_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(prog_SRC ${prog_SRCS})
    cmake_path(GET prog_SRC STEM prog_NAME)
    add_executable(${prog_NAME} ${prog_SRC})
    set_target_properties(${prog_NAME} PROPERTIES
        VISIBILITY_INLINES_HIDDEN ON
        C_VISIBILITY_PRESET "hidden"
        CXX_VISIBILITY_PRESET "hidden")
    if(LINK_STATIC_GCORE)
        target_link_libraries(${prog_NAME} PRIVATE gcore_static)
    else()
        target_link_libraries(${prog_NAME} PRIVATE gcore_shared)
        set_target_properties(${prog_NAME} PROPERTIES
            BUILD_RPATH "$<IF:${APPLE},@executable_path,$ORIGIN>/../lib"
            INSTALL_RPATH "$<IF:${APPLE},@executable_path,$ORIGIN>/../../lib")
    endif()
    install(TARGETS ${prog_NAME} DESTINATION "bin/tests")
endforeach()

add_library(testmodule MODULE "${CMAKE_CURRENT_SOURCE_DIR}/modules/module.cpp")
set_target_properties(testmodule PROPERTIES
    VISIBILITY_INLINES_HIDDEN ON
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden"
    MACOSX_RPATH ON)
if(NOT WIN32)
    set_target_properties(testmodule PROPERTIES PREFIX "")
    if(APPLE)
        set_target_properties(testmodule PROPERTIES SUFFIX ".bundle")
    endif()
endif()
install(TARGETS testmodule DESTINATION "bin/tests")
