cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME mwm)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

include(FindFreetype)
find_package(X11 REQUIRED)
find_package(Freetype REQUIRED)

include_directories(${X11_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})

file(GLOB source_files
	"src/**/*.cpp"
	"src/*.cpp"
	"config.cpp"
)

set(SOURCES ${source_files})

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES} X11 Xft)
