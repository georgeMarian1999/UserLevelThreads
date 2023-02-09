cmake_minimum_required(VERSION 3.24)
project(UserLevelThreads C)

set(CMAKE_C_STANDARD 11)

add_library(UserLevelThreads library.c main.c)
