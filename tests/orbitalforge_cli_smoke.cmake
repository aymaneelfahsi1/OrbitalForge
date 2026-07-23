cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED ORBITALFORGE_EXECUTABLE)
    message(FATAL_ERROR "ORBITALFORGE_EXECUTABLE is required")
endif()

if(NOT DEFINED ORBITALFORGE_TEST_WORK_DIR)
    message(FATAL_ERROR "ORBITALFORGE_TEST_WORK_DIR is required")
endif()

function(require_file path)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "expected file does not exist: ${path}")
    endif()
endfunction()

function(require_contains text pattern)
    string(FIND "${text}" "${pattern}" found_index)

    if(found_index EQUAL -1)
        message(FATAL_ERROR "expected text to contain: ${pattern}")
    endif()
endfunction()

file(REMOVE_RECURSE "${ORBITALFORGE_TEST_WORK_DIR}")
file(MAKE_DIRECTORY "${ORBITALFORGE_TEST_WORK_DIR}")

set(scenario_path "${ORBITALFORGE_TEST_WORK_DIR}/cli_smoke.orbit")
set(output_directory "${ORBITALFORGE_TEST_WORK_DIR}/run")
set(missing_scenario_path "${ORBITALFORGE_TEST_WORK_DIR}/missing.orbit")

file(WRITE "${scenario_path}" "name = CLI smoke\n")
file(APPEND "${scenario_path}" "gravitational_constant = 1.0\n")
file(APPEND "${scenario_path}" "softening = 0.0\n")
file(APPEND "${scenario_path}" "time_step = 0.01\n")
file(APPEND "${scenario_path}" "step_count = 6\n")
file(APPEND "${scenario_path}" "output_interval = 3\n")
file(APPEND "${scenario_path}" "integrator = velocity-verlet\n")
file(APPEND "${scenario_path}" "seed = 42\n")
file(APPEND "${scenario_path}" "body = Primary,1.0,-0.5,0.0,0.0,0.0,-0.5,0.0\n")
file(APPEND "${scenario_path}" "body = Secondary,1.0,0.5,0.0,0.0,0.0,0.5,0.0\n")

execute_process(
    COMMAND "${ORBITALFORGE_EXECUTABLE}" simulate "${scenario_path}" --output "${output_directory}"
    RESULT_VARIABLE simulate_result
    OUTPUT_VARIABLE simulate_stdout
    ERROR_VARIABLE simulate_stderr
)

if(NOT simulate_result EQUAL 0)
    message(FATAL_ERROR "simulate failed with ${simulate_result}: ${simulate_stderr}")
endif()

require_contains("${simulate_stdout}" "Simulation complete")
require_file("${output_directory}/trajectory.csv")
require_file("${output_directory}/diagnostics.csv")
require_file("${output_directory}/metadata.txt")

file(READ "${output_directory}/trajectory.csv" trajectory_csv)
file(READ "${output_directory}/diagnostics.csv" diagnostics_csv)
file(READ "${output_directory}/metadata.txt" metadata_text)

require_contains("${trajectory_csv}" "step,time_seconds,body_name,mass,position_x,position_y,position_z,velocity_x,velocity_y,velocity_z")
require_contains("${trajectory_csv}" "Primary")
require_contains("${trajectory_csv}" "Secondary")
require_contains("${diagnostics_csv}" "step,time_seconds,total_energy,relative_energy_drift,momentum_x,momentum_y,momentum_z,momentum_drift,center_of_mass_x,center_of_mass_y,center_of_mass_z,center_of_mass_drift")
require_contains("${metadata_text}" "application=OrbitalForge")
require_contains("${metadata_text}" "scenario=CLI smoke")
require_contains("${metadata_text}" "integrator=velocity-verlet")

execute_process(
    COMMAND "${ORBITALFORGE_EXECUTABLE}" compare "${scenario_path}"
    RESULT_VARIABLE compare_result
    OUTPUT_VARIABLE compare_stdout
    ERROR_VARIABLE compare_stderr
)

if(NOT compare_result EQUAL 0)
    message(FATAL_ERROR "compare failed with ${compare_result}: ${compare_stderr}")
endif()

require_contains("${compare_stdout}" "OrbitalForge integrator comparison")
require_contains("${compare_stdout}" "explicit-euler")
require_contains("${compare_stdout}" "velocity-verlet")
require_contains("${compare_stdout}" "rk4")

execute_process(
    COMMAND "${ORBITALFORGE_EXECUTABLE}" benchmark "${scenario_path}" --sizes 2,4
    RESULT_VARIABLE benchmark_result
    OUTPUT_VARIABLE benchmark_stdout
    ERROR_VARIABLE benchmark_stderr
)

if(NOT benchmark_result EQUAL 0)
    message(FATAL_ERROR "benchmark failed with ${benchmark_result}: ${benchmark_stderr}")
endif()

require_contains("${benchmark_stdout}" "OrbitalForge benchmark")
require_contains("${benchmark_stdout}" "Bodies")
require_contains("${benchmark_stdout}" "Milliseconds/step")

execute_process(
    COMMAND "${ORBITALFORGE_EXECUTABLE}" simulate "${missing_scenario_path}"
    RESULT_VARIABLE missing_result
    OUTPUT_VARIABLE missing_stdout
    ERROR_VARIABLE missing_stderr
)

if(NOT missing_result EQUAL 3)
    message(FATAL_ERROR "missing scenario returned ${missing_result}")
endif()

require_contains("${missing_stderr}" "could not open scenario file")

execute_process(
    COMMAND "${ORBITALFORGE_EXECUTABLE}" unknown
    RESULT_VARIABLE unknown_result
    OUTPUT_VARIABLE unknown_stdout
    ERROR_VARIABLE unknown_stderr
)

if(NOT unknown_result EQUAL 1)
    message(FATAL_ERROR "unknown command returned ${unknown_result}")
endif()

require_contains("${unknown_stderr}" "Unknown command")
