find_package(doctest CONFIG REQUIRED)

add_executable(test_core_functions_cxx test_core_cxx.cpp)
target_link_libraries(test_core_functions_cxx PRIVATE
        Liara::Interfaces
        Liara::Core
        doctest::doctest)
add_test(NAME test_core_functions_cxx COMMAND test_core_functions_cxx)
