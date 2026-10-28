/**
 * @file bms_app.h
 * @brief Milestone 1 application entry: probe devices, emit boot/error logs.
 */
#ifndef BMS_APP_H
#define BMS_APP_H

#include <stdint.h>

#include "bms_types.h"
#include "ltc4162.h"
#include "max17320.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    BMS_State state;
    MAX17320_Identity gauge;
    LTC4162_Identity charger;
    uint8_t gauge_ok;
    uint8_t charger_ok;
    uint8_t recovery_ok_count;
    uint32_t last_retry_ms;
} BMS_AppContext;

void BMS_App_Init(BMS_AppContext *ctx);

/** Boot probe. Stamps last_retry_ms to now_ms so the next check waits one interval. */
void BMS_App_OnBoot(BMS_AppContext *ctx, uint32_t now_ms);

/**
 * Periodic service. Health-checks ICs while idle (BMS_POLL_INTERVAL_MS) and
 * retries missing ICs every BMS_IC_RETRY_INTERVAL_MS. Leaving FAULT requires
 * BMS_RECOVERY_SUCCESS_CYCLES consecutive good probes. FAULT does not
 * electrically isolate the pack.
 */
void BMS_App_Service(BMS_AppContext *ctx, uint32_t now_ms);

BMS_State BMS_App_GetState(const BMS_AppContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* BMS_APP_H */
