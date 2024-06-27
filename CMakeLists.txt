cmake_minimum_required(VERSION 3.22)

project (CppNativeWinApp)

set (CMAKE_CXX_STANDARD 20)
set (CPP_NATIVE_WIN_APP_LIB cpp_native_win_app)
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/)

# native window lib
file (GLOB_RECURSE NATIVE_WIN_WINDOW_SRC ./src/*.cpp)

add_library                 (${CPP_NATIVE_WIN_APP_LIB} STATIC ${NATIVE_WIN_WINDOW_SRC})
target_include_directories  (${CPP_NATIVE_WIN_APP_LIB} PUBLIC ./include/)

# test proj
option (ENABLE_NWA_TEST OFF)

function(AddTest name)
    file (GLOB_RECURSE TEST_SRC ./test/${name}/*.cpp)
    add_executable          (${name} ${TEST_SRC})
    target_link_libraries   (${name} PRIVATE ${CPP_NATIVE_WIN_APP_LIB})
endfunction()

if (ENABLE_NWA_TEST)
    AddTest(TestWindowStyle)
    AddTest(TestWindowOpenGL)
    AddTest(TestWindowVulkan)
endif ()

