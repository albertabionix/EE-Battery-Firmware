/**
 * @file mock_i2c.c
 */
#include "mock_i2c.h"

static BMS_Status kind_to_status(MockI2C_ResponseKind kind)
{
    switch (kind) {
    case MOCK_I2C_RESP_OK: return BMS_OK;
    case MOCK_I2C_RESP_NACK: return BMS_ERR_NACK;
    case MOCK_I2C_RESP_TIMEOUT: return BMS_ERR_TIMEOUT;
    case MOCK_I2C_RESP_BUS: return BMS_ERR_BUS;
    default: return BMS_ERR_BUS;
    }
}

void MockI2C_Reset(MockI2C_State *state)
{
    size_t i;
    if (state == 0) {
        return;
    }
    for (i = 0; i < MOCK_I2C_MAX_REGS; i++) {
        state->regs[i].valid = 0U;
    }
    state->script_len = 0U;
    state->script_pos = 0U;
    state->read_count = 0U;
    state->write_count = 0U;
}

void MockI2C_SetReg(MockI2C_State *state, uint16_t addr_hal, uint8_t reg, uint16_t value)
{
    size_t i;
    if (state == 0) {
        return;
    }
    for (i = 0; i < MOCK_I2C_MAX_REGS; i++) {
        if (state->regs[i].valid &&
            (state->regs[i].addr_hal == addr_hal) &&
            (state->regs[i].reg == reg)) {
            state->regs[i].value = value;
            return;
        }
    }
    for (i = 0; i < MOCK_I2C_MAX_REGS; i++) {
        if (!state->regs[i].valid) {
            state->regs[i].addr_hal = addr_hal;
            state->regs[i].reg = reg;
            state->regs[i].value = value;
            state->regs[i].valid = 1U;
            return;
        }
    }
}

void MockI2C_ClearScript(MockI2C_State *state)
{
    if (state == 0) {
        return;
    }
    state->script_len = 0U;
    state->script_pos = 0U;
}

int MockI2C_AddScript(MockI2C_State *state, BMS_I2C_Bus bus, uint16_t addr_hal,
                      uint8_t reg, uint8_t is_write, MockI2C_ResponseKind kind,
                      uint16_t value)
{
    MockI2C_ScriptEntry *e;
    if ((state == 0) || (state->script_len >= MOCK_I2C_MAX_SCRIPT)) {
        return 0;
    }
    e = &state->script[state->script_len++];
    e->bus = bus;
    e->addr_hal = addr_hal;
    e->reg = reg;
    e->is_write = is_write;
    e->kind = kind;
    e->value = value;
    return 1;
}

static int find_reg(const MockI2C_State *state, uint16_t addr_hal, uint8_t reg,
                    uint16_t *out)
{
    size_t i;
    for (i = 0; i < MOCK_I2C_MAX_REGS; i++) {
        if (state->regs[i].valid &&
            (state->regs[i].addr_hal == addr_hal) &&
            (state->regs[i].reg == reg)) {
            *out = state->regs[i].value;
            return 1;
        }
    }
    return 0;
}

static BMS_Status mock_read16(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                              uint8_t reg, uint16_t *out_value)
{
    MockI2C_State *state = (MockI2C_State *)ctx;
    uint16_t stored = 0U;

    if ((state == 0) || (out_value == 0)) {
        return BMS_ERR_INVALID_ARG;
    }
    state->read_count++;

    if (state->script_pos < state->script_len) {
        const MockI2C_ScriptEntry *e = &state->script[state->script_pos];
        if ((e->bus == bus) && (e->addr_hal == dev_addr_hal) &&
            (e->reg == reg) && (e->is_write == 0U)) {
            state->script_pos++;
            if (e->kind != MOCK_I2C_RESP_OK) {
                return kind_to_status(e->kind);
            }
            *out_value = e->value;
            return BMS_OK;
        }
    }

    if (find_reg(state, dev_addr_hal, reg, &stored)) {
        *out_value = stored;
        return BMS_OK;
    }
    return BMS_ERR_NACK;
}

static BMS_Status mock_write16(void *ctx, BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                               uint8_t reg, uint16_t value)
{
    MockI2C_State *state = (MockI2C_State *)ctx;

    if (state == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    state->write_count++;

    if (state->script_pos < state->script_len) {
        const MockI2C_ScriptEntry *e = &state->script[state->script_pos];
        if ((e->bus == bus) && (e->addr_hal == dev_addr_hal) &&
            (e->reg == reg) && (e->is_write != 0U)) {
            state->script_pos++;
            return kind_to_status(e->kind);
        }
    }

    MockI2C_SetReg(state, dev_addr_hal, reg, value);
    return BMS_OK;
}

BMS_I2C_Transport MockI2C_MakeTransport(MockI2C_State *state)
{
    BMS_I2C_Transport t;
    t.read16 = mock_read16;
    t.write16 = mock_write16;
    t.ctx = state;
    return t;
}
