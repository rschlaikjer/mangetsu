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

# Put UI stuff behind a build flag so that people don't have to mess around
# with deps as much by default
if (${BUILD_GUI})
  # OpenGL
  include(FindOpenGL)
  include_directories(${OPENGL_INCLUDE_DIRS})

  # Glfw for windowing
  find_package(PkgConfig)
  pkg_check_modules(PC_LIBGLFW REQUIRED glfw3)
  include_directories(${PC_LIBGLFW_INCLUDE_DIRS})
endif()

include_directories(include)

# Json library
include_directories(vendor/json)

# Only build imgui lib if UI is enabled
if (${BUILD_GUI})
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
endif()

add_library(mg_util
  src/util/fs.cpp
)

add_library(mg_data
  src/data/mzp.cpp
  src/data/mzx.cpp
  src/data/mrg.cpp
  src/data/nam.cpp
  src/data/nxx.cpp
)
target_link_libraries(mg_data
    z
    mg_util
)

add_executable(mangetsu
  src/mangetsu.cpp
)
target_link_libraries(mangetsu
  mg_data
)

add_executable(nxx_decompress
    src/tools/nxx_decompress.cpp
)
target_link_libraries(nxx_decompress
    mg_data
)

add_executable(nxgx_compress
    src/tools/nxgx_compress.cpp
)
target_link_libraries(nxgx_compress
    mg_data
)

add_executable(mzx_decompress
    src/tools/mzx_decompress.cpp
)
target_link_libraries(mzx_decompress
    mg_data
)

add_executable(mzx_compress
    src/tools/mzx_compress.cpp
)
target_link_libraries(mzx_compress
    mg_data
)

add_executable(mzp_info
    src/tools/mzp_info.cpp
)
target_link_libraries(mzp_info
    mg_data
)

add_executable(mzp_extract
    src/tools/mzp_extract.cpp
)
target_link_libraries(mzp_extract
    mg_data
    stdc++fs
)

add_executable(mzp_compress
    src/tools/mzp_compress.cpp
)
target_link_libraries(mzp_compress
    mg_data
)

add_executable(nam_read
    src/tools/nam_read.cpp
)
target_link_libraries(nam_read
    mg_data
)

if (${BUILD_GUI})
  add_executable(data_explorer
      src/tools/data_explorer.cpp
  )
  target_link_libraries(data_explorer
      mg_data
      imgui
  )
endif()

add_executable(script_text_to_content_json
    src/tools/script_text_to_content_json.cpp
)
target_link_libraries(script_text_to_content_json
    mg_data
    ssl
    crypto
)

add_executable(repack_script_text_translation
    src/tools/repack_script_text_translation.cpp
)
target_link_libraries(repack_script_text_translation
    mg_data
    ssl
    crypto
)

add_executable(mrg_extract
    src/tools/mrg_extract.cpp
)
target_link_libraries(mrg_extract
    mg_data
    stdc++fs
)

add_executable(mrg_pack
    src/tools/mrg_pack.cpp
)
target_link_libraries(mrg_pack
    mg_data
    stdc++fs
)

add_executable(mrg_info
    src/tools/mrg_info.cpp
)
target_link_libraries(mrg_info
    mg_data
    stdc++fs
)

add_executable(mrg_replace
    src/tools/mrg_replace.cpp
)
target_link_libraries(mrg_replace
    mg_data
    stdc++fs
)
