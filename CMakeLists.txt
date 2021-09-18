cmake_minimum_required(VERSION 3.1)
project(mangetsu LANGUAGES C CXX)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/;/usr/local/")

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4 -g3 -g -rdynamic")
  set(GCC_OPTIMIZATION "-O0")
endif ()

set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Werror -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=pragmas -Wno-error=unused-local-typedefs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_OPTIMIZATION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

# Need openGL
include(FindOpenGL)
include_directories(${OPENGL_INCLUDE_DIRS})

# Glfw for windowing
find_package(PkgConfig)
pkg_check_modules(PC_LIBGLFW REQUIRED glfw3)
include_directories(${PC_LIBGLFW_INCLUDE_DIRS})


include_directories(include)

include_directories(vendor/imgui/)
add_library(imgui
    STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl2.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
)
set_target_properties(imgui
    PROPERTIES POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(imgui
   ${OPENGL_LIBRARIES}
   ${PC_LIBGLFW_LIBRARIES}
)

add_library(mg_data
  src/data/mzp.cpp
  src/data/mzx.cpp
  src/data/nam.cpp
)

add_executable(mangetsu
  src/mangetsu.cpp
)
target_link_libraries(mangetsu
  mg_data
)

add_executable(mzx_decompress
    src/tools/mzx_decompress.cpp
)
target_link_libraries(mzx_decompress
    mg_data
)

add_executable(mzp_info
    src/tools/mzp_info.cpp
)
target_link_libraries(mzp_info
    mg_data
)

add_executable(mzp_roundtrip
    src/tools/mzp_roundtrip.cpp
)
target_link_libraries(mzp_roundtrip
    mg_data
)

add_executable(nam_read
    src/tools/nam_read.cpp
)
target_link_libraries(nam_read
    mg_data
)

add_executable(data_explorer
    src/tools/data_explorer.cpp
)
target_link_libraries(data_explorer
    mg_data
    imgui
)
