cmake_minimum_required(VERSION 3.15)
project(OS2)

set(CMAKE_CXX_STANDARD 14)

add_executable(OS2 base.c stage1.c stage1.h Queue.c Queue.h)