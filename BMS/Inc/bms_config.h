/**
 * @file bms_config.h
 * @brief Compile-time BMS settings (intervals, board constants, addresses).
 *
 * Threshold numeric limits are intentionally absent until approved.
 */
#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Timing (ms) */
#define BMS_POLL_INTERVAL_MS              500U
#define BMS_TELEMETRY_INTERVAL_MS         1000U
#define BMS_IC_RETRY_INTERVAL_MS          1000U
#define BMS_STALE_DATA_LIMIT_MS           1500U
#define BMS_RECOVERY_SUCCESS_CYCLES       2U
#define BMS_I2C_TIMEOUT_MS                50U
#define BMS_FAULT_LED_PERIOD_MS           1000U

/* Pack / sense resistors (from schematic — unverified on assembled board) */
#define BMS_SERIES_CELLS                  4U
#define BMS_PACK_CAPACITY_MAH             4500U
#define BMS_MAX17320_R_CS_MOHM            2.0f
#define BMS_LTC4162_R_SNSB_MOHM           4.0f
#define BMS_LTC4162_R_SNSIN_MOHM          8.0f

/*
 * I2C device addresses for STM32 HAL (write-form / left-shifted).
 *
 * MAX17320 datasheet Table 116 lists slave address bytes 6Ch / 16h (8-bit write
 * form). STM32 HAL expects that same write-form value, so use 0x6C / 0x16.
 * BMS_FIRMWARE_SPEC.md listed 7-bit 0x6C → HAL 0xD8; that double-shifts the
 * Maxim write-form address and is treated as a documentation discrepancy.
 *
 * LTC4162-F datasheet lists 7-bit 0b1101000 (0x68); HAL write-form is 0xD0.
 */
#define BMS_MAX17320_ADDR_MAIN_HAL        0x6CU
#define BMS_MAX17320_ADDR_NV_HAL          0x16U
#define BMS_LTC4162_ADDR_HAL              0xD0U

/* Feature gates — Milestone 1 is read-only monitoring foundation */
#define BMS_FEATURE_DEVICE_WRITES_ENABLED 0
#define BMS_FEATURE_CHARGE_CONTROL        0
#define BMS_FEATURE_CELL_BALANCING        0
#define BMS_FEATURE_THRESHOLD_EVAL        0

#define BMS_FW_VERSION_MAJOR              0U
#define BMS_FW_VERSION_MINOR              1U
#define BMS_FW_VERSION_PATCH              0U

#define BMS_TELEMETRY_QUEUE_DEPTH         16U
#define BMS_TELEMETRY_LINE_MAX            160U

#ifdef __cplusplus
}
#endif

#endif /* BMS_CONFIG_H */
