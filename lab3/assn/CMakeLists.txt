cmake_minimum_required(VERSION 3.7)
project(mdriver)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -g")

set(SOURCE_FILES
        clock.o
        fcyc.o
        fsecs.o
        ftimer.o
        mdriver.o
        memlib.h
        memlib.o
        mm.c
        mm.h)

add_executable(mdriver ${SOURCE_FILES})
