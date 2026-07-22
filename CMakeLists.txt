cmake_minimum_required(VERSION 3.25)

project(
    OrbitalForge
    VERSION 0.1.0
    DESCRIPTION "A modern C++ orbital dynamics workbench"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(
    orbitalforge_core
    src/app/scenario.cpp
    src/math/vec3.cpp
    src/physics/body.cpp
    src/physics/gravity.cpp
    src/physics/integrator.cpp
    src/physics/diagnostics.cpp
    src/simulation/step.cpp
    src/simulation/integrator_kind.cpp
    src/simulation/simulation_runner.cpp
    src/app/scenario_parser.cpp
)

target_include_directories(
    orbitalforge_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    orbitalforge_core
    PUBLIC
        cxx_std_20
)

target_compile_options(
    orbitalforge_core
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
)

add_executable(
    orbitalforge
    apps/orbitalforge_cli/main.cpp
)

target_link_libraries(
    orbitalforge
    PRIVATE
        orbitalforge_core
)

target_compile_options(
    orbitalforge
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
)

add_executable(
    orbitalforge_integrator_comparison
    examples/integrator_comparison.cpp
)

target_link_libraries(
    orbitalforge_integrator_comparison
    PRIVATE
        orbitalforge_core
)

target_compile_options(
    orbitalforge_integrator_comparison
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
)

include(CTest)

if(BUILD_TESTING)
    find_package(Catch2 REQUIRED)

    add_executable(
        orbitalforge_tests
        tests/test_vec3.cpp
        tests/test_body.cpp
        tests/test_gravity.cpp
        tests/test_integrator.cpp
        tests/test_system_state.cpp
        tests/test_diagnostics.cpp
        tests/test_simulation_step.cpp
        tests/test_advanced_integrators.cpp
        tests/test_integrator_kind.cpp
        tests/test_simulation_runner.cpp
        tests/test_scenario.cpp
        tests/test_scenario_parser.cpp
    )

    target_link_libraries(
        orbitalforge_tests
        PRIVATE
            orbitalforge_core
            Catch2::Catch2WithMain
    )

    target_compile_options(
        orbitalforge_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
    )

    include(Catch)
    catch_discover_tests(orbitalforge_tests)
endif()