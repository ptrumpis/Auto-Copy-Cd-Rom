cmake_minimum_required(VERSION 3.23)
project(AutoCopyCdRom)

set(CMAKE_CXX_STANDARD 14)

add_executable(AutoCopyCdRom WIN32 AutoCopyCdRom.cpp CINIFile.cpp AutoCopyCdRom.rc)
