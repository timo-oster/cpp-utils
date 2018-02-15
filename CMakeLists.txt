# At LEAST 2.8 but newer is better
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(cpp_utils VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNODEFAULTLIB)
    add_definitions(-DVC_EXTRALEAN)
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}")

# todo: optionally enable -march=native and measure difference
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic -Wall -Wextra -Wconversion")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3")
endif()

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

find_package(Boost REQUIRED)

# Add header-only library
add_library(cpp_utils INTERFACE)

# Specify compile features for target to get transitive compiler/C++ standard
# version requirement for projects using this target
target_compile_features(cpp_utils INTERFACE
                        cxx_auto_type
                        cxx_constexpr
                        cxx_decltype_incomplete_return_types
                        cxx_decltype
                        cxx_generalized_initializers
                        cxx_range_for
                        cxx_return_type_deduction
                        cxx_trailing_return_types
                        cxx_variable_templates)

# Define headers for this library. INTERFACE headers will be added to
# consumers' build paths.
target_include_directories(cpp_utils INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Boost_INCLUDE_DIR})

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS cpp_utils EXPORT cpp_utilsConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT cpp_utilsConfig
        NAMESPACE cpp_utils::
        DESTINATION share/cpp_utils/cmake)
add_library(cpp_utils::cpp_utils ALIAS cpp_utils)

# This makes the project importable from the build directory
export(TARGETS cpp_utils FILE Cpp_UtilsConfig.cmake)

option(BUILD_TESTS "Build the unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(src)