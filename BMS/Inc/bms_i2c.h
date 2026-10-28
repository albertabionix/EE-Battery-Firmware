/**
 * @file bms_i2c.h
 * @brief Small mockable I2C transport shared by MAX17320 and LTC4162 drivers.
 */
#ifndef BMS_I2C_H
#define BMS_I2C_H

#include <stddef.h>
#include <stdint.h>

#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transport hooks. Production binds HAL; host tests bind a mock.
 * Addresses are STM32-HAL write-form (7-bit << 1).
 * Register payloads are LSB-first 16-bit values for both devices.
 */
typedef struct {
    BMS_Status (*read16)(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                         uint8_t reg, uint16_t *out_value);
    BMS_Status (*write16)(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                          uint8_t reg, uint16_t value);
    void *ctx;
} BMS_I2C_Transport;

void BMS_I2C_SetTransport(const BMS_I2C_Transport *transport);
const BMS_I2C_Transport *BMS_I2C_GetTransport(void);

/** Bind the STM32 HAL backend (no-op / stub under BMS_HOST). */
void BMS_I2C_UseHalTransport(void);

BMS_Status BMS_I2C_Read16(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                          uint8_t reg, uint16_t *out_value);
BMS_Status BMS_I2C_Write16(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                           uint8_t reg, uint16_t value);

const char *BMS_I2C_BusName(BMS_I2C_Bus bus);
const char *BMS_StatusName(BMS_Status status);

#ifdef __cplusplus
}
#endif

#endif /* BMS_I2C_H */
