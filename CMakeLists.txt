cmake_minimum_required(VERSION 3.4)
project(PhasingCalibration  VERSION 1.0
                            LANGUAGES C)

find_package(GSL REQUIRED)

add_executable(phasing_calibration main.c _open_PLX9050.c
        common_functions.c
        _prog_conventions.c utils.c)
target_link_libraries(phasing_calibration m)

