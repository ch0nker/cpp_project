cmake_minimum_required(VERSION 3.10)

project(cpp_project
        VERSION 1.0.0
        DESCRIPTION "Template c++ project creator"
        LANGUAGES C)

add_executable(cpp_project src/main.c src/args.c)