cmake_minimum_required(VERSION 3.4)
project(PhasingCalibration  VERSION 1.0
                            LANGUAGES C)

find_package(GSL REQUIRED)

add_executable(phasing_calibration phasing_calibration.c _open_PLX9050.c
        phasing_cards.c utils.c vna_functions.c MSI_functions.c)
target_link_libraries(phasing_calibration m)
target_compile_options(phasing_calibration PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

add_executable(atten_calibration atten_calibration.c _open_PLX9050.c
        phasing_cards.c _prog_conventions.c utils.c vna_functions.c MSI_functions.c)
target_link_libraries(atten_calibration m)

add_executable(time_delay time_delay.c)
target_link_libraries(time_delay m)

add_executable(summary_generator summary_generator.c)

add_executable(beamcode_generator beamcode_generator.c)
target_link_libraries(beamcode_generator m)

add_executable(write_final_beamcodes write_final_beamcodes.c phasing_cards.c
        _open_PLX9050.c _prog_conventions.c utils.c)
target_link_libraries(write_final_beamcodes m)

add_executable(optimize_beam_programming optimize_beam_programming.c MSI_functions.c
        vna_functions.c utils.c phasing_cards.c _open_PLX9050.c)
target_link_libraries(optimize_beam_programming m)
target_compile_options(optimize_beam_programming PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

add_executable(card_path_test _open_PLX9050.c
        phasing_cards.c _prog_conventions.c utils.c)
target_link_libraries(card_path_test m)