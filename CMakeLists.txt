# cpp-periphery - A C++11 library for peripheral I/O (GPIO, SPI, I2C, MMIO, Serial) in Linux.
#
#  Copyright 2018 - Mark Butowski
#
#  Use, modification and distribution is subject to the
#  MIT License. (See accompanying file LICENSE)
#
#  Project home: https://github.com/mpb27/cpp-periphery
#
#

cmake_minimum_required(VERSION 3.6)
project(cpp-periphery)

# Should set a global c++ standard, or use a per project standards, what are the ABI ramifications?
# set(CMAKE_CXX_STANDARD 11)

# Global warning flags for the entire project.
add_compile_options(-Wall -Wextra -Werror -Wpedantic)

# Change -O3 to -O2 for releases to get a slightly smaller size.
# TODO: Change this global -O2 to a local specific to periphery target only.
string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

# Create a static library from the source files.
add_library(periphery STATIC
        src/periphery/i2c.cpp
        src/periphery/mmio.cpp
        src/periphery/serial.cpp
        src/periphery/spi.cpp
        src/periphery/chardevice.cpp)

#
target_include_directories(periphery PUBLIC  include/)
target_include_directories(periphery PRIVATE src/)

# Set position independed.
set_property(TARGET periphery PROPERTY POSITION_INDEPENDENT_CODE ON)

# Set required language features.
target_compile_features(periphery
        PUBLIC
            cxx_strong_enums
            cxx_variadic_templates
        PRIVATE
            cxx_auto_type
            cxx_constexpr
            cxx_range_for
            cxx_static_assert
        )

# Add alias.
add_library(periphery::periphery ALIAS periphery)

# Test executables.
if (PERIPHERY_TESTS)

    # test-i2c
    add_executable(test-i2c src/test/test-i2c.cpp)
    target_link_libraries(test-i2c PRIVATE periphery::periphery)

    # test-spi
    add_executable(test-spi src/test/test-spi.cpp)
    target_link_libraries(test-spi PRIVATE periphery::periphery)

    # test-mmio
    add_executable(test-mmio src/test/test-mmio.cpp)
    target_link_libraries(test-mmio PRIVATE periphery::periphery)

    # test-serial
    add_executable(test-serial src/test/test-serial.cpp)
    target_link_libraries(test-serial PRIVATE periphery::periphery)

endif()

