cmake_minimum_required(VERSION 3.10)
project(tapi C)

# this does NOT have to be 17, tapi is compatiable with c99
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# source files
file(GLOB_RECURSE TAPI_SOURCES "src/*.c")
file(GLOB_RECURSE TAPI_HEADERS "include/*.h" "src/int/*.h")

# library targ for making a .so
add_library(tapi SHARED ${TAPI_SOURCES})
target_include_directories(tapi PUBLIC include PRIVATE src)

# unit and integration source files
file(GLOB UNIT_TEST_SOURCES "tests/unit/*.c")
file(GLOB INTEGRATION_TEST_SOURCES "tests/integration/*.c")

# add each test source as a executable and link with the .so
foreach(test_src ${UNIT_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} tapi)
    target_include_directories(${test_name} PRIVATE include src)
endforeach()
foreach(test_src ${INTEGRATION_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(int_${test_name} ${test_src})
    target_link_libraries(int_${test_name} tapi)
    target_include_directories(int_${test_name} PRIVATE include)
endforeach()

# documentation gen with doxygen
find_package(Doxygen REQUIRED)
set(AWESOME_CSS_DIR "${CMAKE_SOURCE_DIR}/vendor/doxygen-awesome-css")
configure_file(
        "${CMAKE_SOURCE_DIR}/doc/doxyfile.in"
        "${CMAKE_BINARY_DIR}/doc/Doxyfile"
        @ONLY
)
add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/doc/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "generating doxygen documentation"
        VERBATIM
)