cmake_minimum_required(VERSION 3.8)
project(mdriver)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -O1 -g")

set(SOURCE_FILES
        clock.o
        fcyc.o
        fsecs.o
        ftimer.o
        Makefile
        mdriver
        mdriver.o
        memlib.h
        memlib.o
        mm.c
        mm.h
        README
        short1-bal.rep
        short2-bal.rep)

add_executable(mdriver ${SOURCE_FILES})