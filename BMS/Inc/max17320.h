/**
 * @file max17320.h
 * @brief Minimal read-only MAX17320 connection / status checks (Milestone 1).
 *
 * Register map verified against MAX17320 datasheet / UG7161:
 *   Status  0x000, DevName 0x021 on slave address byte 0x6C (HAL).
 * Device writes / NVM programming are disabled stubs.
 */
#ifndef MAX17320_H
#define MAX17320_H

#include <stdint.h>

#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX17320_REG_STATUS   0x00U
#define MAX17320_REG_DEVNAME  0x21U

typedef struct {
    uint16_t status;
    uint16_t dev_name;
    uint8_t  present;
} MAX17320_Identity;

BMS_Status MAX17320_ReadStatus(uint16_t *out_status);
BMS_Status MAX17320_ReadDevName(uint16_t *out_dev_name);

/**
 * Reads Status + DevName. Succeeds only if both reads work and DevName is one
 * of 0x4209 / 0x420A / 0x420B (datasheet). Does not mutate *out on failure.
 */
BMS_Status MAX17320_CheckDevice(MAX17320_Identity *out);

/** Configuration / NVM write stub — never touches the bus. */
BMS_Status MAX17320_WriteConfig(uint8_t reg, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* MAX17320_H */
