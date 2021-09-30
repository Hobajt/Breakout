cmake_minimum_required(VERSION 3.16)

option(BREAKOUT_GLFW_FROM_SOURCE "GLFW library from sources or installed library." ON)
option(BREAKOUT_GLAD_AS_STATIC_LIB "Link Glad stuff as a separate lib." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Breakout)

add_executable(main 
    "src/main.cpp" "include/breakout/log.h" "include/breakout/gl_debug.h" "src/gl_debug.cpp" "include/breakout/glm.h" "include/breakout/window.h" "src/window.cpp" "include/breakout/gameobject.h" "include/breakout/shader.h" "src/shader.cpp" "include/breakout/resources.h" "src/resources.cpp" "include/breakout/utils.h" "src/utils.cpp")

target_include_directories(main PUBLIC include)

#==== GLFW ====
if(BREAKOUT_GLFW_FROM_SOURCE)
    message(STATUS "===GLFW from sources===")
    add_subdirectory(vendor/glfw)
else()
    message(STATUS "===GLFW from install===")
    find_package(glfw3 3.3 REQUIRED)
endif()
target_link_libraries(main PUBLIC glfw)

#==== Glad ====
if(BREAKOUT_GLAD_AS_STATIC_LIB)
    message(STATUS "===Glad as library===")
    add_subdirectory(vendor/glad)
    target_link_libraries(main PUBLIC glad)
else()
    message(STATUS "===Glad as includes===")
    target_include_directories(main PUBLIC vendor/glad/include)
    target_sources(main PRIVATE vendor/glad/src/glad.cpp)
endif()

#==== GLM ====
target_include_directories(main PUBLIC vendor/glm/include)

#==== stb_image =====
target_include_directories(main PUBLIC vendor/stb_image/include)

#==== FreeType ====
add_subdirectory(vendor/freetype)
target_link_libraries(main PUBLIC freetype)

message(STATUS "===Generated with config types: ${CMAKE_CONFIGURATION_TYPES}===")
