/**
 * @file ltc4162.h
 * @brief Minimal read-only LTC4162-F connection / status checks (Milestone 1).
 *
 * Registers from LTC4162-F datasheet:
 *   charger_state 0x34, SYSTEM_STATUS_REG 0x39, CHEM_CELLS_REG 0x43.
 * 7-bit address 0x68 → HAL 0xD0. Configuration writes disabled.
 */
#ifndef LTC4162_H
#define LTC4162_H

#include <stdint.h>

#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LTC4162_REG_CHARGER_STATE   0x34U
#define LTC4162_REG_SYSTEM_STATUS   0x39U
#define LTC4162_REG_CHEM_CELLS      0x43U

/* CHEM_CELLS_REG.chem enums (datasheet) */
#define LTC4162_CHEM_FAD            4U /* LTC4162-FAD LiFePO4 adjustable */

typedef struct {
    uint16_t chem_cells;
    uint16_t charger_state;
    uint16_t system_status;
    uint8_t  chem;
    uint8_t  cell_count;
    uint8_t  present;
} LTC4162_Identity;

BMS_Status LTC4162_ReadChargerState(uint16_t *out_state);
BMS_Status LTC4162_ReadSystemStatus(uint16_t *out_status);
BMS_Status LTC4162_ReadChemCells(uint16_t *out_chem_cells);

/**
 * Reads CHEM_CELLS, charger_state, and SYSTEM_STATUS. Requires FAD chemistry
 * (chem == LTC4162_CHEM_FAD / 4). Wrong chem returns BMS_ERR_DEVICE.
 * cell_count may read 0 without input power (datasheet). Does not mutate
 * *out on any failure.
 */
BMS_Status LTC4162_CheckDevice(LTC4162_Identity *out);

/** Configuration write stub — never touches the bus. */
BMS_Status LTC4162_WriteConfig(uint8_t reg, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* LTC4162_H */
