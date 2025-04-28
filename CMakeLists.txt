cmake_minimum_required(VERSION 3.10)
project(MeuProjetoOpenCV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "C:/MinGW/mingw64/bin/g++.exe")
set(CMAKE_PREFIX_PATH "C:/SDL2/SDL2/cmake;C:/SDL2/SDL2_mixer/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mconsole")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("./include")
include_directories("C:/SDL2/SDL2/x86_64-w64-mingw32/include")
include_directories("C:/SDL2/SDL2_mixer/x86_64-w64-mingw32/include")

link_directories("C:/SDL2/SDL2/x86_64-w64-mingw32/lib")
link_directories("C:/SDL2/SDL2_mixer/x86_64-w64-mingw32/lib")

add_definitions(-DSDL_MAIN_HANDLED)

add_executable(main main.cpp)

target_link_libraries(main
    ${OpenCV_LIBS}
    SDL2main
    SDL2
    SDL2_mixer
)
