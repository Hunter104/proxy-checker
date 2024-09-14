cmake_minimum_required(VERSION 3.30)
project(proxy_checker)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREADS_FLAG ON)
find_package(Threads REQUIRED)

add_executable(proxy_checker src/main.c ./src/proxy.c ./src/memory.c)
target_include_directories(proxy_checker PRIVATE include)
target_link_libraries(proxy_checker PRIVATE curl cjson Threads::Threads)
