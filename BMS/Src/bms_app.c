/**
 * @file bms_app.c
 * @brief Milestone 1 boot probe, IC health checks, and diagnostic FAULT recovery.
 *
 * FAULT is a software diagnostic state only — it does not electrically isolate
 * charge/discharge paths.
 */
#include "bms_app.h"

#include <stdio.h>

#include "bms_config.h"
#include "bms_i2c.h"
#include "telemetry.h"

static uint32_t elapsed_ms(uint32_t now, uint32_t then)
{
    return now - then;
}

static void apply_probe_result(BMS_AppContext *ctx)
{
    if (ctx->gauge_ok && ctx->charger_ok) {
        if (ctx->state == BMS_STATE_FAULT) {
            if (ctx->recovery_ok_count < 255U) {
                ctx->recovery_ok_count++;
            }
            if (ctx->recovery_ok_count >= BMS_RECOVERY_SUCCESS_CYCLES) {
                ctx->recovery_ok_count = 0U;
                ctx->state = BMS_STATE_IDLE;
                (void)Telemetry_LogEvent("STATE", "IDLE");
            }
            return;
        }

        /* STARTUP (and any non-FAULT path): first healthy probe enters IDLE. */
        ctx->recovery_ok_count = 0U;
        if (ctx->state != BMS_STATE_IDLE) {
            ctx->state = BMS_STATE_IDLE;
            (void)Telemetry_LogEvent("STATE", "IDLE");
        }
        return;
    }

    ctx->recovery_ok_count = 0U;
    if (ctx->state != BMS_STATE_FAULT) {
        ctx->state = BMS_STATE_FAULT;
        (void)Telemetry_LogEvent("STATE", "FAULT_SW_DIAG");
    }
}

static void probe_devices(BMS_AppContext *ctx, uint8_t log_ok)
{
    char detail[96];
    BMS_Status gst;
    BMS_Status cst;

    gst = MAX17320_CheckDevice(&ctx->gauge);
    ctx->gauge_ok = (gst == BMS_OK) ? 1U : 0U;
    if (gst != BMS_OK) {
        (void)snprintf(detail, sizeof(detail), "MAX17320 check failed:%s",
                       BMS_StatusName(gst));
        (void)Telemetry_LogEvent("IC_PROBE", detail);
    } else if (log_ok != 0U) {
        (void)snprintf(detail, sizeof(detail),
                       "MAX17320 ok DevName=0x%04X Status=0x%04X",
                       (unsigned)ctx->gauge.dev_name,
                       (unsigned)ctx->gauge.status);
        (void)Telemetry_LogEvent("IC_PROBE", detail);
    }

    cst = LTC4162_CheckDevice(&ctx->charger);
    ctx->charger_ok = (cst == BMS_OK) ? 1U : 0U;
    if (cst != BMS_OK) {
        (void)snprintf(detail, sizeof(detail), "LTC4162 check failed:%s",
                       BMS_StatusName(cst));
        (void)Telemetry_LogEvent("IC_PROBE", detail);
    } else if (log_ok != 0U) {
        (void)snprintf(detail, sizeof(detail),
                       "LTC4162 ok chem=%u cells=%u state=0x%04X",
                       (unsigned)ctx->charger.chem,
                       (unsigned)ctx->charger.cell_count,
                       (unsigned)ctx->charger.charger_state);
        (void)Telemetry_LogEvent("IC_PROBE", detail);
    }

    apply_probe_result(ctx);
}

void BMS_App_Init(BMS_AppContext *ctx)
{
    if (ctx == 0) {
        return;
    }
    ctx->state = BMS_STATE_STARTUP;
    ctx->gauge_ok = 0U;
    ctx->charger_ok = 0U;
    ctx->recovery_ok_count = 0U;
    ctx->last_retry_ms = 0U;
    ctx->gauge.status = 0U;
    ctx->gauge.dev_name = 0U;
    ctx->gauge.present = 0U;
    ctx->charger.chem_cells = 0U;
    ctx->charger.charger_state = 0U;
    ctx->charger.system_status = 0U;
    ctx->charger.chem = 0U;
    ctx->charger.cell_count = 0U;
    ctx->charger.present = 0U;
}

void BMS_App_OnBoot(BMS_AppContext *ctx, uint32_t now_ms)
{
    if (ctx == 0) {
        return;
    }
    ctx->state = BMS_STATE_STARTUP;
    ctx->recovery_ok_count = 0U;
    (void)Telemetry_LogBoot("M1_readonly_monitor");
    probe_devices(ctx, 1U);
    ctx->last_retry_ms = now_ms;
}

void BMS_App_Service(BMS_AppContext *ctx, uint32_t now_ms)
{
    uint32_t interval_ms;
    uint8_t recovering;

    if (ctx == 0) {
        return;
    }

    (void)Telemetry_Service(4U);

    /* FAULT / missing-IC path uses retry interval; healthy IDLE uses poll. */
    recovering = (ctx->state == BMS_STATE_FAULT) ||
                 (ctx->gauge_ok == 0U) || (ctx->charger_ok == 0U);
    interval_ms = (recovering != 0U) ? BMS_IC_RETRY_INTERVAL_MS
                                     : BMS_POLL_INTERVAL_MS;

    if (elapsed_ms(now_ms, ctx->last_retry_ms) < interval_ms) {
        return;
    }
    ctx->last_retry_ms = now_ms;
    if (recovering != 0U) {
        (void)Telemetry_LogEvent("RETRY", "probing_ics");
    }
    /* Quiet healthy IDLE polls; always log failures inside probe_devices. */
    probe_devices(ctx, recovering);
}

BMS_State BMS_App_GetState(const BMS_AppContext *ctx)
{
    if (ctx == 0) {
        return BMS_STATE_FAULT;
    }
    return ctx->state;
}
