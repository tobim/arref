cmake_minimum_required(VERSION 3.0)

add_library(arref INTERFACE)
target_include_directories(arref INTERFACE include/)
target_sources(arref INTERFACE include/arref/arref.hpp)
