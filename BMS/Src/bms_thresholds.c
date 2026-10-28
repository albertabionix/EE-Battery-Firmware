/**
 * @file bms_thresholds.c
 * @brief Unconfigured threshold placeholders — never imply a pass.
 */
#include "bms_thresholds.h"

BMS_Status BMS_Thresholds_EvaluateCellVoltage(float cell_v)
{
    (void)cell_v;
    return BMS_ERR_NOT_CONFIGURED;
}

BMS_Status BMS_Thresholds_EvaluatePackCurrent(float pack_a)
{
    (void)pack_a;
    return BMS_ERR_NOT_CONFIGURED;
}

BMS_Status BMS_Thresholds_EvaluateTemperature(float temp_c)
{
    (void)temp_c;
    return BMS_ERR_NOT_CONFIGURED;
}
