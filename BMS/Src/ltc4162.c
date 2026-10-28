/**
 * @file ltc4162.c
 * @brief Read-only LTC4162-F identity / status helpers.
 */
#include "ltc4162.h"

#include "bms_config.h"
#include "bms_i2c.h"
#include "telemetry.h"

BMS_Status LTC4162_ReadChargerState(uint16_t *out_state)
{
    BMS_Status st;

    if (out_state == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    st = BMS_I2C_Read16(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                        LTC4162_REG_CHARGER_STATE, out_state);
    if (st != BMS_OK) {
        (void)Telemetry_LogI2cFailure(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                                      LTC4162_REG_CHARGER_STATE, st);
    }
    return st;
}

BMS_Status LTC4162_ReadSystemStatus(uint16_t *out_status)
{
    BMS_Status st;

    if (out_status == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    st = BMS_I2C_Read16(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                        LTC4162_REG_SYSTEM_STATUS, out_status);
    if (st != BMS_OK) {
        (void)Telemetry_LogI2cFailure(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                                      LTC4162_REG_SYSTEM_STATUS, st);
    }
    return st;
}

BMS_Status LTC4162_ReadChemCells(uint16_t *out_chem_cells)
{
    BMS_Status st;

    if (out_chem_cells == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    st = BMS_I2C_Read16(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                        LTC4162_REG_CHEM_CELLS, out_chem_cells);
    if (st != BMS_OK) {
        (void)Telemetry_LogI2cFailure(BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                                      LTC4162_REG_CHEM_CELLS, st);
    }
    return st;
}

BMS_Status LTC4162_CheckDevice(LTC4162_Identity *out)
{
    LTC4162_Identity local;
    BMS_Status st;

    if (out == 0) {
        return BMS_ERR_INVALID_ARG;
    }

    local.chem_cells = 0U;
    local.charger_state = 0U;
    local.system_status = 0U;
    local.chem = 0U;
    local.cell_count = 0U;
    local.present = 0U;

    /* CHEM_CELLS_REG is always readable and identifies the populated variant. */
    st = LTC4162_ReadChemCells(&local.chem_cells);
    if (st != BMS_OK) {
        return st;
    }

    st = LTC4162_ReadChargerState(&local.charger_state);
    if (st != BMS_OK) {
        return st;
    }

    /* SYSTEM_STATUS is a critical charger status read for Milestone 1. */
    st = LTC4162_ReadSystemStatus(&local.system_status);
    if (st != BMS_OK) {
        return st;
    }

    local.chem = (uint8_t)((local.chem_cells >> 8) & 0x0FU);
    local.cell_count = (uint8_t)(local.chem_cells & 0x0FU);

    /* FADM expected chem=4. Reject clearly wrong chemistry families. */
    if (local.chem != LTC4162_CHEM_FAD) {
        return BMS_ERR_DEVICE;
    }

    local.present = 1U;
    *out = local;
    return BMS_OK;
}

BMS_Status LTC4162_WriteConfig(uint8_t reg, uint16_t value)
{
    (void)reg;
    (void)value;
#if BMS_FEATURE_DEVICE_WRITES_ENABLED
#error "Device writes must not be enabled in Milestone 1"
#else
    return BMS_ERR_WRITE_DISABLED;
#endif
}
