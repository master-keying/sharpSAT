cmake_minimum_required (VERSION 3.5)
project (sharpSAT VERSION 1.2 LANGUAGES CXX)

option(SHARPSAT_BUILD_TESTING "Build and run sharpSAT's tests" ON)

include(GNUInstallDirs)

add_library(libsharpSAT STATIC
    src/alt_component_analyzer.cpp
    src/component_analyzer.cpp
    src/component_cache.cpp
    src/component_management.cpp
    src/instance.cpp
    src/new_component_analyzer.cpp
    src/solver.cpp
    src/statistics.cpp
    src/stopwatch.cpp
    src/structures.cpp
    src/component_types/base_packed_component.cpp
    src/component_types/component_archetype.cpp
    # headers
    include/sharpSAT/alt_component_analyzer.h
    include/sharpSAT/component_analyzer.h
    include/sharpSAT/component_cache.h
    include/sharpSAT/component_cache-inl.h
    include/sharpSAT/component_management.h
    include/sharpSAT/containers.h
    include/sharpSAT/instance.h
    include/sharpSAT/new_component_analyzer.h
    include/sharpSAT/primitive_types.h
    include/sharpSAT/solver_config.h
    include/sharpSAT/solver.h
    include/sharpSAT/stack.h
    include/sharpSAT/statistics.h
    include/sharpSAT/stopwatch.h
    include/sharpSAT/structures.h
    include/sharpSAT/unions.h
    include/sharpSAT/component_types/base_packed_component.h
    include/sharpSAT/component_types/cacheable_component.h
    include/sharpSAT/component_types/component_archetype.h
    include/sharpSAT/component_types/component.h
    include/sharpSAT/component_types/difference_packed_component.h
    include/sharpSAT/component_types/simple_packed_component.h
    include/sharpSAT/component_types/simple_unpacked_component.h
    include/sharpSAT/containers/binary_heap.h
)

# Keep the library named as either libsharpSAT.a or sharpSAT.lib
# While having the target's logical name be distinct from sharpSAT (the binary)
set_target_properties(libsharpSAT
    PROPERTIES
      OUTPUT_NAME "sharpSAT"
)

## Add a namespace alias.
# This is useful to abstract over use of the library as installed vs subdirectory build
add_library(sharpSAT::libsharpSAT ALIAS libsharpSAT)

target_compile_features(libsharpSAT
    PUBLIC
      cxx_attributes
      cxx_defaulted_functions
      cxx_deleted_functions
      cxx_final
)

target_include_directories(libsharpSAT
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (NOT WIN32)
    find_library(GMP_LIB gmp)
    find_library(GMPXX_LIB gmpxx)
endif()

target_link_libraries(libsharpSAT PUBLIC ${GMP_LIB} ${GMPXX_LIB})


# On Windows we use MPIR via vcpkg and it is not integrated with
# CMake at all. Thus we have to hack something together for now.
if (WIN32)
    target_include_directories(libsharpSAT PUBLIC "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/")
    target_link_libraries(libsharpSAT PUBLIC "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/mpir.lib")
endif()
# The current code is a mess and abuses MSVC iterators in an invalid way

add_executable(sharpSAT src/main.cpp)
target_link_libraries(sharpSAT libsharpSAT)



if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( libsharpSAT PRIVATE -Wall -Wextra )
    target_compile_options( sharpSAT PRIVATE -Wall -Wextra )
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( libsharpSAT PRIVATE /W4 /wd4127 /wd4244 )
    target_compile_options( sharpSAT PRIVATE /W4 /wd4127 /wd4244 )
endif()

# Keep build optimized -- Only for GCC/Clang, MSVC does Debug/Release differently
if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( libsharpSAT PRIVATE -O3 )
    target_compile_options( sharpSAT PRIVATE -O3 )
endif()

# Workaround for libstdc++ + Clang + -std=gnu++11 bug.
set_target_properties(libsharpSAT sharpSAT
    PROPERTIES
      CXX_EXTENSIONS OFF
)



#########
# Tests
##
include(CTest)

if (SHARPSAT_BUILD_TESTING AND BUILD_TESTING)

    # Test the timeout CLI option
    add_test(NAME integration:cli-timeout
        COMMAND sharpSAT -t 1 "test/benchmark/pmc/circuit/3bitadd_32.cnf"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )
    set_tests_properties(integration:cli-timeout PROPERTIES
        PASS_REGULAR_EXPRESSION "^TIMEOUT\n$" )
    set_tests_properties(integration:cli-timeout PROPERTIES
        TIMEOUT 3 ) # 3s test timeout, sharpSAT should end in 1s

    # Read all easy instances from a file
    file(READ "${PROJECT_SOURCE_DIR}/test/benchmark/easy.txt" SHARPSAT_AUTORUN_TESTS)
    string(REGEX REPLACE ";" "\\\\;" SHARPSAT_AUTORUN_TESTS "${SHARPSAT_AUTORUN_TESTS}")
    string(REGEX REPLACE "\n" ";" SHARPSAT_AUTORUN_TESTS "${SHARPSAT_AUTORUN_TESTS}")

    # Add 1 test for each easy instance
    foreach(AUTORUN_TEST ${SHARPSAT_AUTORUN_TESTS})
        add_test(NAME "benchmark:${AUTORUN_TEST}"
            COMMAND sharpSAT "test/benchmark/${AUTORUN_TEST}.cnf"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        ) # Initialize the test
        file(READ "test/benchmark/${AUTORUN_TEST}.txt" EXPECTED_MODEL_COUNT)
        set_tests_properties("benchmark:${AUTORUN_TEST}" PROPERTIES
            PASS_REGULAR_EXPRESSION "^${EXPECTED_MODEL_COUNT}$"
        ) # Set the expected output
        set_tests_properties("benchmark:${AUTORUN_TEST}" PROPERTIES
            TIMEOUT 30
        ) # 30s timeout
    endforeach(AUTORUN_TEST)

endif( SHARPSAT_BUILD_TESTING AND BUILD_TESTING )


###############
# Installation
##
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/sharpSAT)

install(
    TARGETS
      libsharpSAT
    EXPORT sharpSAT-Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    TARGETS
      sharpSAT
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h*")

install(EXPORT sharpSAT-Targets
    FILE sharpSAT-Targets.cmake
    NAMESPACE sharpSAT::
    DESTINATION ${INSTALL_CONFIGDIR}
)


#####################
# ConfigVersion file
##
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/CMake/sharpSATConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

## Install all the helper files
install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/sharpSATConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT sharpSAT-Targets FILE ${CMAKE_CURRENT_BINARY_DIR}/sharpSAT-Targets.cmake NAMESPACE sharpSAT::)
export(PACKAGE sharpSAT)
