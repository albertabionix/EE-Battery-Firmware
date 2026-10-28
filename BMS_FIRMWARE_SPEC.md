# Bionix BMS Firmware — Engineering Specification

**Status:** Planning baseline / pre-hardware bring-up  
**Updated:** 2026-10-28  
**Target:** STM32G030K8T6, STM32 HAL, C  
**Authority:** This document records agreed project requirements. Hardware values are transcribed from the supplied `BMS_Schematic_Oct10.pdf` (October 10 schematic/BOM) and the user's “Pin Configurations For BMS” Google Doc. Do not assume assembled-board operation has been validated.

## 1. Goals and boundaries

Build a modular, readable **monitoring-first** firmware foundation for a four-series-cell LiFePO4 battery system. The STM32 reads a MAX17320 fuel gauge/protector and an LTC4162-F charger over separate I2C buses, reports data via UART CSV, identifies software/hardware faults, and performs diagnostics. Initial firmware must not change charging or battery protection configuration.

The assembled PCB is reported to match the provided schematic, **but has not yet been powered, programmed, or tested**. Hardware safety, firmware compilation, and mock test success are distinct milestones.

### Explicit safety gate

- Battery: **Bioenno Power BLF-12045W**, marketed 12 V, 4.5 Ah LiFePO4; 4S assumed/confirmed by project information. Typical 4S LFP nominal voltage is approximately 12.8 V. Verify cell manufacturer charging limits and actual external-cell connections before energizing.
- The Bioenno commercial pack may include an internal PCM/BMS. Confirm how individual cell taps are actually provided and whether the pack's internal protection is retained. Never infer safe access to individual cells just from a drawn schematic.
- **MAX17320G20+T chemistry/model/protection configuration must be independently assessed for this exact 4S LiFePO4 application before active use.** Do not assume factory-programmed values are appropriate. The IC's fuel-gauge capacity, protections, and SOH/SOC estimates are not yet validated.
- Charger: **LTC4162EUFD-FADM#PBF** variant indicated in schematic; verify exact populated device, LiFePO4 configuration, resistor scaling, and charging profile before enabling any charge path.
- No software setting substitutes for independent hardware protection; no critical protection or charging configuration writes during the initial milestone.
- Firmware `FAULT` is a **software diagnostic state**, not a proven electrical isolation mechanism. When hardware control is not verified, do not claim that `FAULT` disables charging/discharging.

## 2. Hardware inventory (schematic/BOM)

| Role | Designator | Part / value | Notes |
|---|---|---|---|
| MCU | U3 | STM32G030K8T6 | 32-pin package, 64 KB flash |
| Fuel gauge/protector | U2 | MAX17320G20+T | 2S–4S gauge/protection IC; model verification outstanding |
| Battery charger | U1 | LTC4162EUFD-FADM#PBF | Verify actual configuration |
| MCU supply | U4 | RBT05W24S3V3 | Nominal 3.3 V converter |
| Gauge sense resistor | R_CS | **2 mΩ** | Not 5 mΩ; use in gauge conversions |
| Charger battery sense | R_SNSB | **4 mΩ** | For charger battery-current scaling |
| Charger input sense | R_SNSIN | **8 mΩ** | For charger input-current scaling |
| Cell-balancing resistors | R_BAL1..4 | **30 Ω each** | Hardware path and effective controls to verify |
| Gauge thermistors | R_TH1..4 | 10 kΩ NTC each | Datasheet/BOM indicates B57861S0103F040 |
| Charger thermistor | R_TH5 | 10 kΩ NTC | Separate charger NTC |
| Power path switches | Q1,Q2,Q4,Q5 | IRLR3915TRPBF | Function/wiring must be inspected before controlling |
| Status LED | D2 | RGB | PB0/PB1/PB2 |

PCB schematic notes mention possible future precharge circuitry and a charging input connector. The supplied PDF may represent a design snapshot; user reports assembled board matches it, but power-on is pending.

## 3. MCU connections and peripheral configuration

| MCU pins | Interface / signal | Associated component |
|---|---|---|
| PA9 / PA10 | I2C1 SCL / SDA | MAX17320, `SCL1` / `SDA1` |
| PA11 / PA12 | I2C2 SCL / SDA | LTC4162-F, `SCL2` / `SDA2` |
| PB6 / PB7 | USART1 TX / RX | J1 debug UART |
| PA13 / PA14 | SWDIO / SWCLK | J3 programming via ST-LINK |
| NRST | Reset | J3 |
| PA1 | ALRT / provisional EXTI1 | Verify polarity, pull-up and source |
| PA3 | SMBALERT / provisional EXTI3 | Verify polarity, pull-up and source |
| PC6 | BMSALERT / provisional EXTI6 | Verify polarity, pull-up and source |
| PB0 / PB1 / PB2 | RED / GREEN / BLUE | RGB LED, polarity check at bring-up |
| PA4..PA7 | SPI1 signals | Optional J2 header, not used by battery ICs |

Initial CubeMX settings: HSI 16 MHz system clock, AHB/APB divided by 1, I2C1 and I2C2 each targeting 100 kHz, USART1 115200 baud 8N1. Use CubeMX-generated timing values and re-check actual `.ioc`/pin availability. SWD must remain enabled. Schematic shows I2C pull-ups (4.7 kΩ entries); verify actual physical circuit.

### Addresses — verify against specific datasheet revision and on board

| Device | Expected 7-bit address | STM32 HAL shifted value |
|---|---|---|
| MAX17320 main register space | `0x6C` | `0xD8` |
| MAX17320 extended register space | `0x16` | `0x2C` |
| LTC4162-F | `0x68` | `0xD0` |

These are **expected datasheet addresses**, not experimentally detected devices. Use proper 7-bit vs HAL-shifted conventions and device-specific register transaction formats; do not blindly use generic 16-bit memory addressing. The MAX17320 unique chip identifier, if required, must be obtained from the actual device with its documented interface; it is not supplied by the datasheet.

## 4. Development environment and source tree

Keep all existing CubeMX-generated files and conventional project setup intact. User edits primarily in **VS Code**, uses **STM32CubeMX** for pin/peripheral generation, and may use STM32CubeIDE toolchain for target build/debug. No RTOS initially.

```text
Existing STM32 project root/
├── Core/                     # generated: preserve
├── Drivers/                  # CMSIS/HAL generated: preserve
├── Startup/                  # preserve
├── <existing .ioc/build files>
├── BMS/                      # new application code
│   ├── Inc/
│   │   ├── bms_config.h
│   │   ├── bms_types.h
│   │   ├── bms_i2c.h
│   │   ├── max17320.h
│   │   ├── ltc4162.h
│   │   ├── battery_monitor.h
│   │   ├── charge_manager.h
│   │   ├── fault_manager.h
│   │   ├── bms_thresholds.h
│   │   ├── bms_controller.h
│   │   ├── telemetry.h
│   │   └── status_led.h
│   └── Src/
│       └── matching .c files
└── Tests/                    # host GCC + CMake/CTest, mocks
```

Build system must explicitly include new `.c` and headers; do not assume placing files in `BMS/` automatically compiles them. Preserve `USER CODE` regions if editing generated files; use `.ioc` regeneration for generated peripheral changes. `spi.c` may remain unused.

### Driver style

- **Simple, named measurement functions**, e.g. `BMS_Status MAX17320_ReadPackVoltage(float *out_v)`; `BMS_Status LTC4162_ReadChargeCurrent(float *out_a)`.
- A **small common I2C transport abstraction** under those functions so production STM32 HAL and host mocks can supply the same operations. Keep device-specific register interpretations and conversions within corresponding device `.c` files.
- All read APIs return status and write the output **only upon success**. Preserve units in function names/docs; sign current consistently and verify with device datasheets.
- Detailed errors: `BMS_OK`, timeout, NACK, bus, invalid data, device not ready, write disabled, invalid argument; extend as needed. Map STM32 HAL error conditions carefully.
- **Implement write API stubs now but disable hardware configuration writes** (`BMS_ERR_WRITE_DISABLED`). Distinguish device-protocol address phases from application-level configuration changes; do not assert that every I2C write-shaped transfer is prohibited if it is needed for a read transaction. No nonvolatile programming or protection override without later explicit review.

## 5. Firmware behavior and timing

Five application states: `STARTUP`, `IDLE`, `CHARGING`, `DISCHARGING`, `FAULT`. Determine charging/discharging from validated current polarity and charger status, not raw sign assumptions alone. Software state is diagnostic; dedicated IC protections remain authoritative for actual hardware behavior.

| Activity | Schedule |
|---|---|
| Battery measurements | Every 500 ms |
| Charger measurements and periodic status | Every 500 ms |
| Service alert flags | As soon as practical in main loop |
| CSV data emission | Every 1,000 ms |
| Retry unavailable IC at startup | Every 1,000 ms |
| Stale-data limit | 1,500 ms |
| Recovery from failed monitoring | 2 consecutive successful critical-monitoring cycles, then verify no critical hardware faults |
| LED fault pattern | Red blink at 1 Hz, 500 ms on / 500 ms off |

Use `HAL_GetTick()`/unsigned elapsed-time comparison with no long `HAL_Delay()` in normal loop. GPIO interrupt callbacks only flag work; do not conduct I2C transactions within ISRs. Use bounded I2C/serial operations and account for bus or UART stalls.

### Boot/recovery

1. Initialize HAL/clock/GPIO/I2C1/I2C2/UART and enter `STARTUP`.
2. Read appropriate status/configuration registers to establish device responsiveness; an ACK alone is insufficient.
3. If either IC is not usable, enter `FAULT` and **halt normal processing** (MCU remains alive, UART minimal boot diagnostics). Retry communication at 1 s intervals; do not automatically reset MCU.
4. Resume monitoring automatically after both ICs pass fresh checks, all critical required values are valid, and two consecutive successful polls show no blocking fault. Recovery from communication failure **must not clear hardware protection latches**.
5. No successful initial boot or restart constitutes confirmation of safe hardware protection configuration.

## 6. Telemetry requirements

Must eventually include: cell1..4 V, pack V, signed pack A, SOC %, remaining capacity, SOH if supported, battery temp1..4, charger input voltage/current, charge current/status, fault/protection flags, validity and freshness. Uncalibrated SOC/SOH may be read but must be marked `UNVERIFIED` and not represented as accurate. If unsupported, report `NaN` plus a reason/flag rather than fabricate a percentage.

UART: 115200 8N1. **CSV typed records** `BOOT`, `DATA`, `EVENT` with distinct, explicitly documented fixed schemas. `DATA` at 1 s; fault transitions as `EVENT`; **also record every I2C transaction failure** with source/bus/register/status. Store fault state only in RAM initially; PC saves logs. Prefer an error counter / bounded UART output queue and report dropped records rather than allow a broken I2C device to flood or block the scheduler. Invalid measurements are `NaN` and have validity flags. Suggested validity semantics: `VALID`, `UNVERIFIED`, `INVALID`, `STALE`; do not collapse all measurement groups into one validity bit.

## 7. Faults and thresholds

**Critical when missing/invalid:** four individual cell-voltage readings, pack current, battery temperatures, hardware protection status, charger communication/status. **Noncritical when missing:** SOC and SOH. Failed critical read triggers software `FAULT` promptly; 1.5 s freshness limit independently marks stale values. Fault recovery needs two valid cycles and acceptable hardware status.

Fault sources: gauge protection flags, charger status/fault flags, I2C failures, stale/invalid critical data, and future configurable MCU-side warning/critical comparisons. Severity classes: warning / critical / communication failure. Log set/clear transitions distinctly from per-transaction errors.

Create **unconfigured placeholders** for critical and warning threshold APIs now (cell voltage, current, temperature). Limits are not yet approved: no guessed default values, no implicit pass due to unset values, and no writes to IC protections. Return explicit `NOT_CONFIGURED`/equivalent for absent software thresholds. Distinguish this from a validated `NO_FAULT` result. After limits are approved, update `bms_config.h`, recompile and reflash; no runtime change via UART in v1.

### Configuration header

Central compile-time `BMS/Inc/bms_config.h` to hold 500/1000/1500 ms intervals, 2-cycle recovery, 4S / 4500 mAh, resistor values (2/4/8 mΩ), feature gates, and future threshold settings. Avoid contradicting the device's real register conversions and board revision. Avoid introducing runtime nonvolatile config in initial version.

## 8. Status LED (verify active polarity on board)

- STARTUP: solid yellow
- IDLE: solid green
- CHARGING: solid blue
- DISCHARGING: solid cyan
- FAULT: blinking red at 1 Hz

GPIO only; nonblocking `StatusLED_Update(now_ms)`.

## 9. Register-driver milestone plan

Implement incrementally, checking datasheets for every register address, access width, byte order, signedness, and physical-unit conversion:

1. Both device communication/status/identification checks.
2. Four cell voltages and pack voltage.
3. Pack current using **R_CS=2 mΩ**.
4. SOC and remaining capacity (unverified until model configured).
5. Four gauge thermistors and charger temperature.
6. Charger input voltage, current, charge current, state (charger sense resistors 4/8 mΩ).
7. Hardware protection and alert decoding.
8. SOH/health availability and validity.

Do not invent a device ID register or convert a raw value with an assumed factor. For unknown or unsupported fields, use clear TODOs and errors.

## 10. Testing — required before hardware bring-up

Host GCC plus **plain C tests via CMake/CTest**, no third-party unit test framework initially. Mock low-level I2C with scripted register values and failure outcomes. Test:

- Normal 4S readings and numeric conversion using known datasheet vectors.
- IDLE/CHARGING/DISCHARGING state transitions.
- Protection faults and severity; immediate invalidation on critical read failure.
- Timeout/NACK/bus failure, retry scheduling and 1.5 s stale values.
- Auto-recovery after exactly 2 valid polls, contingent on hardware fault status.
- Typed CSV `BOOT`/`DATA`/`EVENT`, `NaN`, validity indicators, per-failure I2C event logs.
- Disabled write stubs never invoke a write transport.
- Missing configured warning/critical software limits return a distinct unconfigured status.

Mock tests **cannot establish electrical safety, bus integrity, IC factory configuration, cell-tap wiring, or real fault protections**. Hardware bring-up later requires a review of the power source, battery pack access, device settings, connector/polarity, and safe validation procedures by qualified project personnel.

## 11. Milestone 1 — immediate implementation scope

**Deliverable:** Compiling STM32 framework and passing host mock tests for basic read-only device communication.

1. Inspect existing `.ioc`, generated peripheral handles and build configuration; do not replace them.
2. Create `BMS/Inc`, `BMS/Src`, `Tests`; add `bms_config.h`, `bms_types.h`, `bms_i2c.h/.c`.
3. Define a mockable transport for both `hi2c1` and `hi2c2`; preserve correct per-device register-transfer semantics.
4. Add minimal `max17320.h/.c`, `ltc4162.h/.c` with safe read-only status checks based on verified datasheet registers.
5. Add minimal typed CSV boot/error output, bounded and scheduler-friendly.
6. Write PC-host tests for responding device, NACK, timeout, bus error, and disabled writes.
7. Integrate via generated `main.c` `USER CODE` sections or a small BMS entrypoint; ensure target build and host tests pass.
8. Record unverified hardware facts and next steps in project README.

**Out of scope for Milestone 1:** live charging, balancing commands, threshold programming, EEPROM/flash IC configuration, protection overrides, calibrated SOC/SOH, actual hardware performance claims.

## 12. Open verification items

- Actual MAX17320/LTC4162 register map and transfer framing confirmed against manufacturer datasheet revisions.
- MAX17320 factory model/protection configuration **compatibility with 4S LiFePO4**.
- Whether individual cell taps are safely and legitimately available from the commercial battery pack, and interaction with its internal PCM/BMS.
- Exact populated resistor values and pull-ups / alert polarity verified via inspection/measurement when board is ready.
- Charging input connector and charge path; no charge testing until reviewed.
- Threshold values sourced from cell and IC specifications and approved before activation.
- Detailed per-record CSV column schema finalized during implementation.

## 13. Source references

- User-provided `BMS_Schematic_Oct10.pdf`, page 1 schematic and page 2 BOM.
- Google Doc “Pin Configurations For BMS”: https://docs.google.com/document/d/1_3MLW8Pbz1kQjiQkXjwraekwS0nAi4xUVnlwLEELASE/edit
- MAX17320 official datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/max17320.pdf
- LTC4162-F official datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/LTC4162-F.pdf
- Bioenno Power BLF-12045W manufacturer product information: https://www.bioennopower.com/products/12v-4-5ah-lfp-battery-pvc-pack

Datasheet links are references for implementation; always check current revisions and exact orderable part numbers. This file documents decisions, not a completed safety certification.
