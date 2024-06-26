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

if (ENABLE_NWA_TEST)
    add_executable          (test_window_style ./test/TestWindowStyle.cpp)
    target_link_libraries   (test_window_style PRIVATE ${CPP_NATIVE_WIN_APP_LIB})

    add_executable          (test_window_opengl ./test/TestWindowOpenGL.cpp test/glad/gl.cpp)
    target_link_libraries   (test_window_opengl PRIVATE ${CPP_NATIVE_WIN_APP_LIB})
endif ()

