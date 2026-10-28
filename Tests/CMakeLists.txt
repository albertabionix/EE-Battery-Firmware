cmake_minimum_required(VERSION 3.16)
project(bms_host_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(test_milestone1
    test_milestone1.c
    mock_i2c.c
    ../BMS/Src/bms_i2c.c
    ../BMS/Src/max17320.c
    ../BMS/Src/ltc4162.c
    ../BMS/Src/bms_thresholds.c
    ../BMS/Src/telemetry.c
    ../BMS/Src/bms_app.c
)

target_include_directories(test_milestone1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../BMS/Inc
)

target_compile_definitions(test_milestone1 PRIVATE BMS_HOST=1)

enable_testing()
add_test(NAME milestone1 COMMAND test_milestone1)
