cmake_minimum_required(VERSION 3.16)

project(BetterShell VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIOMM REQUIRED giomm-2.68)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.68)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

file(GLOB_RECURSE SOURCE_FILES "src/*.cc")

add_executable(better-shell ${SOURCE_FILES})

target_include_directories(better-shell PRIVATE
    include
    extern/utf8cpp
    extern/utf8cpp/utf8
    ${GIOMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

target_link_libraries(better-shell PRIVATE
    ${GIOMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

target_compile_definitions(better-shell PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    APP_NAME="${PROJECT_NAME}"
    APP_ID="org.BetterDE.Better-Shell"
)

target_compile_options(better-shell PRIVATE
    ${GIOMM_CFLAGS_OTHER} ${GLIBMM_CFLAGS_OTHER} ${JSONCPP_CFLAGS_OTHER}
)
target_link_libraries(better-shell PRIVATE
    ${GIOMM_LDFLAGS} ${GLIBMM_LDFLAGS} ${JSONCPP_LDFLAGS}
)

install(TARGETS better-shell
        RUNTIME DESTINATION bin)