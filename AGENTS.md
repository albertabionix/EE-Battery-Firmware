# Codex Instructions — Bionix BMS Firmware

Read `BMS_FIRMWARE_SPEC.md` before working on this project. Treat it as the current project requirements and safety boundary. **The assembled BMS PCB has not been powered or validated.** Never infer electrical safety from code compilation or mocked tests.

## Scope and workflow

- Work within the **existing STM32CubeMX-generated STM32G030K8T6 project**. Read the `.ioc`, current `Core/`, generated HAL/CMSIS files and build configuration before editing.
- **Do not reorganize or overwrite** `Core/`, `Drivers/CMSIS/`, `Drivers/STM32G0xx_HAL_Driver/`, `Startup/` or generated files. Only edit generated files inside preserved `USER CODE BEGIN/END` regions unless explicitly approved. Changes to pin/peripheral/clock configuration must be driven by `.ioc`/CubeMX or proposed for approval first.
- Add new application code under `BMS/Inc/` and `BMS/Src/`, and host tests under `Tests/`. Explicitly integrate files with the existing target build and include paths.
- Primary workflow: VS Code for coding; STM32CubeMX for generated settings; STM32 HAL for MCU; standard GCC + CMake/CTest for standalone plain-C host unit tests. No RTOS and no additional dependencies unless requested.
- Work **incrementally**. Do Milestone 1 before measurement drivers, fault manager or charger control. Make small coherent edits, run available builds/tests, and report exactly what passed or could not be run.

## Hardware and peripheral constraints

- MCU: STM32G030K8T6. MAX17320G20+T on I2C1 (PA9 SCL, PA10 SDA). LTC4162EUFD-FADM#PBF on I2C2 (PA11 SCL, PA12 SDA). They are **on separate buses**, not a shared bus.
- USART1 PB6 TX/PB7 RX at 115200 8N1; PA13/PA14 are SWD; maintain programming access. LED PB0/PB1/PB2; alert inputs PA1/PA3/PC6; verify polarities at bring-up.
- Schematic resistor values: MAX17320 R_CS **2 mΩ**, charger R_SNSB **4 mΩ**, charger R_SNSIN **8 mΩ**; 4 gauge thermistors + 1 charger thermistor; 4S battery rated 4.5 Ah LiFePO4.
- Verify all addresses, register widths, read framing, signed conversion and scaling against **official manufacturer datasheets** before implementing. Do not invent a device-ID register, value conversion, bit mapping, or protection setting. Clearly label unverified details.

## Absolute safety restrictions for initial firmware

- **Read-only monitoring phase.** Do not implement live charging/discharging commands, cell balancing actuation, protections overrides, threshold changes, EEPROM/NVM programming, or active charger configuration. Write-function interfaces are allowed only as **disabled stubs that return `BMS_ERR_WRITE_DISABLED` and do not perform the device-setting operation**. Device protocol phases needed to read registers must be handled correctly.
- A `FAULT` software state is for reporting/diagnostics; do not claim it electrically disables charge/discharge.
- The MAX17320's factory configuration **has not been validated for a 4S LiFePO4 battery**. Treat SOC/SOH as unverified; do not mark its protection configuration safe by default. The Bioenno commercial battery may contain its own BMS; cell access/interaction need independent confirmation before hardware tests.
- Critical/warning software threshold functions should exist as **unconfigured placeholders**. Never insert arbitrary battery limits or treat `NOT_CONFIGURED` as a passing safety evaluation.
- Do not connect, power, flash, or operate the real PCB, or suggest doing so as an automatic agent action. Hardware bring-up requires separate review and explicit instruction.

## Code conventions

- C, simple readable named functions such as `BMS_Status MAX17320_ReadPackVoltage(float *out_v)`; use a **small shared mockable I2C interface** beneath device-specific measurement functions. Do not over-engineer an elaborate class/layer hierarchy.
- Return detailed errors (timeout, NACK, bus, invalid, not-ready, write-disabled, invalid-argument). Do not mutate measurement outputs on failure. Keep physical-unit conversions documented and testable.
- Use unsigned elapsed-tick scheduling, avoid long delays and blocking UART prints; I2C transfers use bounded timeouts. EXTI callbacks only set flags; I2C work happens in the main loop.
- Poll battery and charger at 500 ms, emit typed CSV `DATA` every 1000 ms, retry unavailable ICs at 1000 ms, treat critical stale data after 1500 ms, and require 2 consecutive good monitoring cycles plus acceptable hardware status for recovery.
- Five states: STARTUP, IDLE, CHARGING, DISCHARGING, FAULT. If an IC is missing at boot, **halt normal BMS monitoring, not the MCU**; continue retry/diagnostics with no automatic reset.
- Safety-critical measurements: four cell voltages, battery current, four temperatures, protection state, charger communication/status. Noncritical: SOC/SOH. Device faults and invalid critical readings can enter FAULT.
- UART logging has typed `BOOT`, `DATA`, `EVENT` CSV records, with `NaN` + validity for unavailable values. Log **every I2C transaction failure**, but use a bounded queue/count dropped logs so failure storms cannot stall monitoring. Emit fault transitions distinctly.
- LED: startup yellow, idle green, charging blue, discharging cyan, fault red blink 1 Hz; no delay-based blink.
- `bms_config.h` is the central source of compile-time settings. No UART runtime reconfiguration in v1.

## Testing requirements

- Add host-only mock I2C and plain C tests via CMake/CTest; isolate STM32 HAL from host builds.
- Include tests for normal operation, charging/discharging classification, fault activation, I2C timeouts/NACK/bus failures, retry behavior, 1.5 s stale data, 2-success recovery, CSV formats/`NaN`/validity, disabled writes, and unconfigured threshold behavior as code becomes available.
- Document exact build and test commands. Do not claim a build or test passed unless it was actually run successfully. Do not claim mocks prove hardware electrical behavior.

## Agent reporting requirements

Before changing code, briefly state which files you intend to edit and why. After editing, summarize changed files, build/test results, pending assumptions and safety-related TODOs. Ask for explicit approval before broad refactors, changing CubeMX pin assignments, enabling write operations, setting protection thresholds, or altering charge control behavior.

## Initial task when first opened

1. Inspect project tree and `.ioc` to verify pin/peripheral setup and build toolchain.
2. Report discrepancies with `BMS_FIRMWARE_SPEC.md` rather than silently fixing them.
3. Implement only Milestone 1: `bms_types.h`, `bms_config.h`, mockable `bms_i2c`, minimal read-only device connection/status checks, bounded boot/error logging, and host tests.
4. Build host tests and STM32 target if toolchains are available; otherwise provide exact commands and explain missing tools.
