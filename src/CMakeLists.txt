
add_subdirectory(lib)

if(GCORE_BUILD_TOOLS)
    add_subdirectory(bin)
endif()

if(GCORE_BUILD_PYTHON)
    add_subdirectory(py)
endif()

if(GCORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
