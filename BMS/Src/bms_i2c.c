/**
 * @file bms_i2c.c
 * @brief I2C transport binding (HAL on target, injectable mock on host).
 */
#include "bms_i2c.h"

#include "bms_config.h"

#ifndef BMS_HOST
#include "i2c.h"
#endif

static const BMS_I2C_Transport *s_transport;

#ifndef BMS_HOST
static I2C_HandleTypeDef *hal_handle_for_bus(BMS_I2C_Bus bus)
{
    if (bus == BMS_I2C_BUS_GAUGE) {
        return &hi2c1;
    }
    if (bus == BMS_I2C_BUS_CHARGER) {
        return &hi2c2;
    }
    return 0;
}

static BMS_Status map_hal_status(HAL_StatusTypeDef st, I2C_HandleTypeDef *hi2c)
{
    if (st == HAL_OK) {
        return BMS_OK;
    }
    if (st == HAL_TIMEOUT) {
        return BMS_ERR_TIMEOUT;
    }
    if (st == HAL_BUSY) {
        return BMS_ERR_NOT_READY;
    }
    if (hi2c != 0) {
        uint32_t err = HAL_I2C_GetError(hi2c);
        if ((err & HAL_I2C_ERROR_AF) != 0U) {
            return BMS_ERR_NACK;
        }
        if ((err & (HAL_I2C_ERROR_BERR | HAL_I2C_ERROR_ARLO |
                    HAL_I2C_ERROR_OVR | HAL_I2C_ERROR_DMA)) != 0U) {
            return BMS_ERR_BUS;
        }
    }
    return BMS_ERR_BUS;
}

static BMS_Status hal_read16(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                             uint8_t reg, uint16_t *out_value)
{
    uint8_t buf[2];
    I2C_HandleTypeDef *hi2c;
    HAL_StatusTypeDef st;

    (void)ctx;
    if (out_value == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    hi2c = hal_handle_for_bus(bus);
    if (hi2c == 0) {
        return BMS_ERR_INVALID_ARG;
    }

    st = HAL_I2C_Mem_Read(hi2c, dev_addr_hal, reg, I2C_MEMADD_SIZE_8BIT,
                          buf, 2U, BMS_I2C_TIMEOUT_MS);
    if (st != HAL_OK) {
        return map_hal_status(st, hi2c);
    }
    *out_value = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return BMS_OK;
}

static BMS_Status hal_write16(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                              uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    I2C_HandleTypeDef *hi2c;
    HAL_StatusTypeDef st;

    (void)ctx;
    hi2c = hal_handle_for_bus(bus);
    if (hi2c == 0) {
        return BMS_ERR_INVALID_ARG;
    }

    buf[0] = (uint8_t)(value & 0xFFU);
    buf[1] = (uint8_t)((value >> 8) & 0xFFU);
    st = HAL_I2C_Mem_Write(hi2c, dev_addr_hal, reg, I2C_MEMADD_SIZE_8BIT,
                           buf, 2U, BMS_I2C_TIMEOUT_MS);
    return map_hal_status(st, hi2c);
}

static const BMS_I2C_Transport s_hal_transport = {
    hal_read16,
    hal_write16,
    0
};
#endif /* !BMS_HOST */

void BMS_I2C_SetTransport(const BMS_I2C_Transport *transport)
{
    s_transport = transport;
}

const BMS_I2C_Transport *BMS_I2C_GetTransport(void)
{
    return s_transport;
}

void BMS_I2C_UseHalTransport(void)
{
#ifndef BMS_HOST
    s_transport = &s_hal_transport;
#else
    s_transport = 0;
#endif
}

BMS_Status BMS_I2C_Read16(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                          uint8_t reg, uint16_t *out_value)
{
    if (out_value == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    if ((s_transport == 0) || (s_transport->read16 == 0)) {
        return BMS_ERR_NOT_READY;
    }
    return s_transport->read16(s_transport->ctx, bus, dev_addr_hal, reg, out_value);
}

BMS_Status BMS_I2C_Write16(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                           uint8_t reg, uint16_t value)
{
    if ((s_transport == 0) || (s_transport->write16 == 0)) {
        return BMS_ERR_NOT_READY;
    }
    return s_transport->write16(s_transport->ctx, bus, dev_addr_hal, reg, value);
}

const char *BMS_I2C_BusName(BMS_I2C_Bus bus)
{
    if (bus == BMS_I2C_BUS_GAUGE) {
        return "I2C1";
    }
    if (bus == BMS_I2C_BUS_CHARGER) {
        return "I2C2";
    }
    return "I2C?";
}

const char *BMS_StatusName(BMS_Status status)
{
    switch (status) {
    case BMS_OK: return "OK";
    case BMS_ERR_TIMEOUT: return "TIMEOUT";
    case BMS_ERR_NACK: return "NACK";
    case BMS_ERR_BUS: return "BUS";
    case BMS_ERR_INVALID: return "INVALID";
    case BMS_ERR_NOT_READY: return "NOT_READY";
    case BMS_ERR_WRITE_DISABLED: return "WRITE_DISABLED";
    case BMS_ERR_INVALID_ARG: return "INVALID_ARG";
    case BMS_ERR_NOT_CONFIGURED: return "NOT_CONFIGURED";
    case BMS_ERR_DEVICE: return "DEVICE";
    default: return "UNKNOWN";
    }
}
