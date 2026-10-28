/**
 * @file bms_thresholds.h
 * @brief Unconfigured placeholder APIs for MCU-side warning/critical limits.
 *
 * Limits are not approved. These APIs must return BMS_ERR_NOT_CONFIGURED and
 * must never be treated as a passing safety evaluation.
 */
#ifndef BMS_THRESHOLDS_H
#define BMS_THRESHOLDS_H

#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

BMS_Status BMS_Thresholds_EvaluateCellVoltage(float cell_v);
BMS_Status BMS_Thresholds_EvaluatePackCurrent(float pack_a);
BMS_Status BMS_Thresholds_EvaluateTemperature(float temp_c);

#ifdef __cplusplus
}
#endif

#endif /* BMS_THRESHOLDS_H */
