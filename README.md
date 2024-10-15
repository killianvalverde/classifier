# classifier

## Table of Contents
- [Introduction](#introduction)
- [Requirements](#requirements)
- [Build & Install](#build--install)
- [Documentation](#documentation)

## Introduction

classifier parses a set of directories and builds a directory tree in which links are placed in 
order to classify them into different categories.
For more information, use the `classifier --help` command.

## Requirements

In order to compile this software you have to use a C++ revision equal or highter to C++20 
(ISO/IEC 14882:2020). Additionally, you must have the following tools and libraries, each with a 
version that is equal to or higher than the specified ones:
- GCC 11.4.0 (sudo apt-get install g++)
- CMake 3.22 (http://www.cmake.org/)
- speed x.y.z (https://github.com/killianvalverde/speed/tree/vx.y.z)

## Build & Install

Use the folowing commands to buil and install the CMake project:

1. Create a directory to hold the build output and generate the native build scripts:

        cmake -H. -Bbuild

2. Compile the project directly from CMake using the native build scripts:

        cmake --build build

3. Install the binary in your environment:

        sudo cmake --install build

## Documentation

In order to learn how to use classifier, feel free to check 
the [documentation](docs/index.md).

If you want to generate Doxygen documentation, you can use the `doxygen Doxyfile` command, and 
the results will be placed in the `./docs` directory.
