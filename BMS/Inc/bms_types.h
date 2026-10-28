/**
 * @file bms_types.h
 * @brief Shared BMS status codes, validity, and application state types.
 */
#ifndef BMS_TYPES_H
#define BMS_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BMS_OK = 0,
    BMS_ERR_TIMEOUT,
    BMS_ERR_NACK,
    BMS_ERR_BUS,
    BMS_ERR_INVALID,
    BMS_ERR_NOT_READY,
    BMS_ERR_WRITE_DISABLED,
    BMS_ERR_INVALID_ARG,
    BMS_ERR_NOT_CONFIGURED,
    BMS_ERR_DEVICE
} BMS_Status;

typedef enum {
    BMS_VALIDITY_VALID = 0,
    BMS_VALIDITY_UNVERIFIED,
    BMS_VALIDITY_INVALID,
    BMS_VALIDITY_STALE
} BMS_Validity;

typedef enum {
    BMS_STATE_STARTUP = 0,
    BMS_STATE_IDLE,
    BMS_STATE_CHARGING,
    BMS_STATE_DISCHARGING,
    BMS_STATE_FAULT
} BMS_State;

typedef enum {
    BMS_I2C_BUS_GAUGE = 0,   /* I2C1 / MAX17320 */
    BMS_I2C_BUS_CHARGER = 1, /* I2C2 / LTC4162-F */
    BMS_I2C_BUS_COUNT
} BMS_I2C_Bus;

#ifdef __cplusplus
}
#endif

#endif /* BMS_TYPES_H */
