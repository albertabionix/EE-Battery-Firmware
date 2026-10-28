/**
 * @file mock_i2c.h
 * @brief Scriptable host-side I2C mock for Milestone 1 tests.
 */
#ifndef MOCK_I2C_H
#define MOCK_I2C_H

#include <stddef.h>
#include <stdint.h>

#include "bms_i2c.h"
#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOCK_I2C_MAX_REGS 64
#define MOCK_I2C_MAX_SCRIPT 32

typedef enum {
    MOCK_I2C_RESP_OK = 0,
    MOCK_I2C_RESP_NACK,
    MOCK_I2C_RESP_TIMEOUT,
    MOCK_I2C_RESP_BUS
} MockI2C_ResponseKind;

typedef struct {
    BMS_I2C_Bus bus;
    uint16_t addr_hal;
    uint8_t reg;
    uint8_t is_write; /* 0=read, 1=write */
    MockI2C_ResponseKind kind;
    uint16_t value; /* used for successful reads if no stored reg */
} MockI2C_ScriptEntry;

typedef struct {
    uint16_t addr_hal;
    uint8_t reg;
    uint16_t value;
    uint8_t valid;
} MockI2C_Reg;

typedef struct {
    MockI2C_Reg regs[MOCK_I2C_MAX_REGS];
    MockI2C_ScriptEntry script[MOCK_I2C_MAX_SCRIPT];
    size_t script_len;
    size_t script_pos;
    uint32_t read_count;
    uint32_t write_count;
} MockI2C_State;

void MockI2C_Reset(MockI2C_State *state);
void MockI2C_SetReg(MockI2C_State *state, uint16_t addr_hal, uint8_t reg, uint16_t value);
void MockI2C_ClearScript(MockI2C_State *state);
int MockI2C_AddScript(MockI2C_State *state, BMS_I2C_Bus bus, uint16_t addr_hal,
                      uint8_t reg, uint8_t is_write, MockI2C_ResponseKind kind,
                      uint16_t value);

BMS_I2C_Transport MockI2C_MakeTransport(MockI2C_State *state);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_I2C_H */
