
file(GLOB gcore_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rex/*.cpp")

file(GLOB gcore_INCS
    "${CMAKE_SOURCE_DIR}/include/gcore/*.h")

add_library(gcore_static STATIC ${gcore_SRCS})

add_library(gcore_shared SHARED ${gcore_SRCS})

target_include_directories(gcore_static
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")

target_compile_options(gcore_static PRIVATE "-W" "-Wall")

target_compile_options(gcore_shared PRIVATE "-W" "-Wall")

target_include_directories(gcore_shared
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")

set_target_properties(gcore_static gcore_shared PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VISIBILITY_INLINES_HIDDEN ON
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden")

if(WIN32)
    set_target_properties(gcore_static PROPERTIES
        OUTPUT_NAME "libgcore")
else()
    set_target_properties(gcore_static PROPERTIES
        OUTPUT_NAME "gcore")
endif()

set_target_properties(gcore_static PROPERTIES
    EXPORT_NAME "static_lib")

target_compile_definitions(gcore_static
    PUBLIC
        "GCORE_STATIC"
    PRIVATE
        "$<$<BOOL:${GCORE_PARSER_VERBOSE}>:GCORE_DEBUG>"
        "$<$<BOOL:${GCORE_REX_VERBOSE}>:_DEBUG_REX>")

target_compile_definitions(gcore_shared
    PRIVATE
        "GCORE_EXPORTS"
        "$<$<BOOL:${GCORE_PARSER_VERBOSE}>:GCORE_DEBUG>"
        "$<$<BOOL:${GCORE_REX_VERBOSE}>:_DEBUG_REX>")

set_target_properties(gcore_shared PROPERTIES
    OUTPUT_NAME "gcore"
    MACOSX_RPATH ON
    EXPORT_NAME "shared_lib"
    VERSION "${gcore_VERSION_MAJOR}.${gcore_VERSION_MINOR}.${gcore_VERSION_PATCH}"
    SOVERSION "${gcore_VERSION_MAJOR}")

set_target_properties(gcore_static gcore_shared PROPERTIES
    INTERFACE_gcore_MAJOR_VERSION "${gcore_VERSION_MAJOR}"
    COMPATIBLE_INTERFACE_STRING "gcore_MAJOR_VERSION")

install(TARGETS gcore_static gcore_shared
    EXPORT gcoreTargets
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib")

install(EXPORT gcoreTargets
    FILE "gcoreTargets.cmake"
    NAMESPACE "gcore::"
    DESTINATION "lib/cmake/gcore")

install(FILES ${gcore_INCS} DESTINATION "include/gcore")

install(FILES "${CMAKE_SOURCE_DIR}/include/half.h" DESTINATION "include")


export(EXPORT gcoreTargets NAMESPACE "gcore::" FILE "${CMAKE_BINARY_DIR}/gcoreTargets.cmake")


# Generate config package
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gcoreConfig.cmake"
    INSTALL_DESTINATION "lib/cmake/gcore")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/gcoreConfigVersion.cmake"
    VERSION "${gcore_VERSION_MAJOR}.${gcore_VERSION_MINOR}.${gcore_VERSION_PATCH}"
    COMPATIBILITY AnyNewerVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/gcoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/gcoreConfigVersion.cmake"
    DESTINATION "lib/cmake/gcore")
