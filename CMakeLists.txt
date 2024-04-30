cmake_minimum_required(VERSION 3.22)

project(CppNativeWinApp)

set(CMAKE_CXX_STANDARD 20)
set(CPP_NATIVE_WIN_APP_LIB cpp_native_win_app)

# native window lib
file (GLOB_RECURSE NATIVE_WIN_WINDOW_SRC src/*.cpp)
add_library(${CPP_NATIVE_WIN_APP_LIB} STATIC
        ${NATIVE_WIN_WINDOW_SRC}
)

target_include_directories(${CPP_NATIVE_WIN_APP_LIB} PUBLIC
        include/
)

# test proj
add_subdirectory(test)
