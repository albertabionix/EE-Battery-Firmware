/**
 * @file max17320.c
 * @brief Read-only MAX17320 identity / status helpers.
 */
#include "max17320.h"

#include "bms_config.h"
#include "bms_i2c.h"
#include "telemetry.h"

static int is_known_devname(uint16_t name)
{
    return (name == 0x4209U) || (name == 0x420AU) || (name == 0x420BU);
}

BMS_Status MAX17320_ReadStatus(uint16_t *out_status)
{
    BMS_Status st;

    if (out_status == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    st = BMS_I2C_Read16(BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                        MAX17320_REG_STATUS, out_status);
    if (st != BMS_OK) {
        (void)Telemetry_LogI2cFailure(BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                                      MAX17320_REG_STATUS, st);
    }
    return st;
}

BMS_Status MAX17320_ReadDevName(uint16_t *out_dev_name)
{
    BMS_Status st;

    if (out_dev_name == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    st = BMS_I2C_Read16(BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                        MAX17320_REG_DEVNAME, out_dev_name);
    if (st != BMS_OK) {
        (void)Telemetry_LogI2cFailure(BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                                      MAX17320_REG_DEVNAME, st);
    }
    return st;
}

BMS_Status MAX17320_CheckDevice(MAX17320_Identity *out)
{
    MAX17320_Identity local;
    BMS_Status st;

    if (out == 0) {
        return BMS_ERR_INVALID_ARG;
    }

    local.status = 0U;
    local.dev_name = 0U;
    local.present = 0U;

    st = MAX17320_ReadDevName(&local.dev_name);
    if (st != BMS_OK) {
        return st;
    }

    st = MAX17320_ReadStatus(&local.status);
    if (st != BMS_OK) {
        return st;
    }

    if (!is_known_devname(local.dev_name)) {
        return BMS_ERR_DEVICE;
    }

    local.present = 1U;
    *out = local;
    return BMS_OK;
}

BMS_Status MAX17320_WriteConfig(uint8_t reg, uint16_t value)
{
    (void)reg;
    (void)value;
#if BMS_FEATURE_DEVICE_WRITES_ENABLED
#error "Device writes must not be enabled in Milestone 1"
#else
    return BMS_ERR_WRITE_DISABLED;
#endif
}
