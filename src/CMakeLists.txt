

if(BUILD_TESTS)
    find_package(doctest REQUIRED)

    # Every library has unit tests, of course
    add_executable(test_cpp_utils
        tests/test_cpp_utils.cpp)

    target_link_libraries(test_cpp_utils
        cpp_utils
        doctest::doctest)

    add_test(test_cpp_utils test_cpp_utils)
endif()
