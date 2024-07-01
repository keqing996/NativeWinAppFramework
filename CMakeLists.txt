cmake_minimum_required(VERSION 3.22)

project (CppNativeWinApp)

set (CMAKE_CXX_STANDARD 20)
set (CPP_NATIVE_WIN_APP_LIB cpp_native_win_app)
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/)

# vulkan
find_package                (Vulkan REQUIRED)

# native window lib
file (GLOB_RECURSE NATIVE_WIN_WINDOW_SRC ./src/*.cpp)

add_library                 (${CPP_NATIVE_WIN_APP_LIB} STATIC ${NATIVE_WIN_WINDOW_SRC})
target_include_directories  (${CPP_NATIVE_WIN_APP_LIB} PUBLIC ./include/ ${Vulkan_INCLUDE_DIRS})
target_link_libraries       (${CPP_NATIVE_WIN_APP_LIB} PUBLIC Vulkan::Vulkan)

# test proj
option (ENABLE_NWA_TEST OFF)

if (ENABLE_NWA_TEST)
    # style test
    add_executable          (TestWindowStyle ./test/TestWindowStyle/Main.cpp)
    target_link_libraries   (TestWindowStyle PRIVATE ${CPP_NATIVE_WIN_APP_LIB})

    # openGL support test
    add_executable          (TestWindowOpenGL ./test/TestWindowOpenGL/Main.cpp ./test/TestWindowOpenGL/glad/gl.cpp)
    target_link_libraries   (TestWindowOpenGL PRIVATE ${CPP_NATIVE_WIN_APP_LIB})

    # vulkan support test
    add_executable              (TestWindowVulkan ./test/TestWindowVulkan/Main.cpp)
    target_link_libraries       (TestWindowVulkan PRIVATE ${CPP_NATIVE_WIN_APP_LIB})
endif ()

