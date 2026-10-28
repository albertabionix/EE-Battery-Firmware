/**
 * @file test_milestone1.c
 * @brief Host unit tests for Milestone 1 read-only device checks.
 */
#include <stdio.h>
#include <string.h>

#include "bms_app.h"
#include "bms_config.h"
#include "bms_i2c.h"
#include "bms_thresholds.h"
#include "ltc4162.h"
#include "max17320.h"
#include "mock_i2c.h"
#include "telemetry.h"

static int g_failures;
static char g_last_line[BMS_TELEMETRY_LINE_MAX];

static void expect_true(const char *name, int cond)
{
    if (!cond) {
        printf("FAIL: %s\n", name);
        g_failures++;
    } else {
        printf("PASS: %s\n", name);
    }
}

static void sink_capture(const char *line, size_t len, void *ctx)
{
    (void)ctx;
    if (len >= sizeof(g_last_line)) {
        len = sizeof(g_last_line) - 1U;
    }
    memcpy(g_last_line, line, len);
    g_last_line[len] = '\0';
}

static void seed_healthy_devices(MockI2C_State *mock)
{
    /* MAX17320 DevName 0x420A, Status arbitrary non-zero */
    MockI2C_SetReg(mock, BMS_MAX17320_ADDR_MAIN_HAL, MAX17320_REG_DEVNAME, 0x420AU);
    MockI2C_SetReg(mock, BMS_MAX17320_ADDR_MAIN_HAL, MAX17320_REG_STATUS, 0x0002U);
    /* LTC4162-FAD chem=4, cell_count=4 (cell_count may be 0 without VIN) */
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHEM_CELLS,
                   (uint16_t)((LTC4162_CHEM_FAD << 8) | 4U));
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHARGER_STATE, 256U);
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_SYSTEM_STATUS, 0x0001U);
}

static void seed_charger_only(MockI2C_State *mock)
{
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHEM_CELLS,
                   (uint16_t)((LTC4162_CHEM_FAD << 8) | 4U));
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHARGER_STATE, 256U);
    MockI2C_SetReg(mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_SYSTEM_STATUS, 1U);
}

static void test_responding_devices(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    MAX17320_Identity gi;
    LTC4162_Identity ci;

    MockI2C_Reset(&mock);
    seed_healthy_devices(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    expect_true("max17320_check_ok", MAX17320_CheckDevice(&gi) == BMS_OK);
    expect_true("max17320_devname", gi.dev_name == 0x420AU);
    expect_true("max17320_present", gi.present == 1U);

    expect_true("ltc4162_check_ok", LTC4162_CheckDevice(&ci) == BMS_OK);
    expect_true("ltc4162_chem_fad", ci.chem == LTC4162_CHEM_FAD);
    expect_true("ltc4162_present", ci.present == 1U);
}

static void test_nack_timeout_bus(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    uint16_t value = 0xA5A5U;
    BMS_Status st;

    MockI2C_Reset(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    MockI2C_AddScript(&mock, BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                      MAX17320_REG_DEVNAME, 0, MOCK_I2C_RESP_NACK, 0);
    st = MAX17320_ReadDevName(&value);
    expect_true("nack_status", st == BMS_ERR_NACK);
    expect_true("nack_preserves_out", value == 0xA5A5U);

    MockI2C_ClearScript(&mock);
    value = 0x1234U;
    MockI2C_AddScript(&mock, BMS_I2C_BUS_GAUGE, BMS_MAX17320_ADDR_MAIN_HAL,
                      MAX17320_REG_STATUS, 0, MOCK_I2C_RESP_TIMEOUT, 0);
    st = MAX17320_ReadStatus(&value);
    expect_true("timeout_status", st == BMS_ERR_TIMEOUT);
    expect_true("timeout_preserves_out", value == 0x1234U);

    MockI2C_ClearScript(&mock);
    value = 0xBEEFU;
    MockI2C_AddScript(&mock, BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                      LTC4162_REG_CHEM_CELLS, 0, MOCK_I2C_RESP_BUS, 0);
    st = LTC4162_ReadChemCells(&value);
    expect_true("bus_status", st == BMS_ERR_BUS);
    expect_true("bus_preserves_out", value == 0xBEEFU);
}

static void test_disabled_writes(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    uint32_t writes_before;

    MockI2C_Reset(&mock);
    seed_healthy_devices(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    writes_before = mock.write_count;
    expect_true("max_write_disabled",
                MAX17320_WriteConfig(0x61U, 0x0000U) == BMS_ERR_WRITE_DISABLED);
    expect_true("ltc_write_disabled",
                LTC4162_WriteConfig(0x14U, 0x0001U) == BMS_ERR_WRITE_DISABLED);
    expect_true("writes_never_hit_bus", mock.write_count == writes_before);
}

static void test_thresholds_unconfigured(void)
{
    expect_true("thr_cell",
                BMS_Thresholds_EvaluateCellVoltage(3.3f) == BMS_ERR_NOT_CONFIGURED);
    expect_true("thr_current",
                BMS_Thresholds_EvaluatePackCurrent(1.0f) == BMS_ERR_NOT_CONFIGURED);
    expect_true("thr_temp",
                BMS_Thresholds_EvaluateTemperature(25.0f) == BMS_ERR_NOT_CONFIGURED);
}

static void test_boot_and_two_cycle_recovery(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    BMS_AppContext app;
    const uint32_t boot_ms = 1000U;

    Telemetry_Init();
    Telemetry_SetSink(sink_capture, 0);
    g_last_line[0] = '\0';

    MockI2C_Reset(&mock);
    /* Only charger present → software FAULT diagnostic */
    seed_charger_only(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    BMS_App_Init(&app);
    BMS_App_OnBoot(&app, boot_ms);
    (void)Telemetry_Service(16U);

    expect_true("boot_state_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);
    expect_true("boot_csv_prefix", strncmp(g_last_line, "BOOT,", 5) == 0 ||
                strstr(g_last_line, "EVENT,") != 0);

    /* Heal gauge */
    MockI2C_SetReg(&mock, BMS_MAX17320_ADDR_MAIN_HAL, MAX17320_REG_DEVNAME, 0x4209U);
    MockI2C_SetReg(&mock, BMS_MAX17320_ADDR_MAIN_HAL, MAX17320_REG_STATUS, 0U);

    /* First consecutive success must remain FAULT */
    BMS_App_Service(&app, boot_ms + BMS_IC_RETRY_INTERVAL_MS);
    expect_true("retry_one_stays_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);

    /* Second consecutive success enters IDLE */
    BMS_App_Service(&app, boot_ms + (2U * BMS_IC_RETRY_INTERVAL_MS));
    expect_true("retry_two_recovers_idle", BMS_App_GetState(&app) == BMS_STATE_IDLE);
}

static void test_recovery_resets_after_fail(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    BMS_AppContext app;
    const uint32_t boot_ms = 2000U;

    Telemetry_Init();
    Telemetry_SetSink(0, 0);

    MockI2C_Reset(&mock);
    seed_charger_only(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    BMS_App_Init(&app);
    BMS_App_OnBoot(&app, boot_ms);
    expect_true("reset_boot_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);

    /* One good cycle */
    seed_healthy_devices(&mock);
    BMS_App_Service(&app, boot_ms + BMS_IC_RETRY_INTERVAL_MS);
    expect_true("reset_one_good_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);

    /* Intervening failure clears the streak */
    MockI2C_Reset(&mock);
    seed_charger_only(&mock);
    BMS_App_Service(&app, boot_ms + (2U * BMS_IC_RETRY_INTERVAL_MS));
    expect_true("reset_after_fail_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);

    /* Need two fresh successes */
    seed_healthy_devices(&mock);
    BMS_App_Service(&app, boot_ms + (3U * BMS_IC_RETRY_INTERVAL_MS));
    expect_true("reset_need_two_a", BMS_App_GetState(&app) == BMS_STATE_FAULT);
    BMS_App_Service(&app, boot_ms + (4U * BMS_IC_RETRY_INTERVAL_MS));
    expect_true("reset_need_two_b", BMS_App_GetState(&app) == BMS_STATE_IDLE);
}

static void test_idle_health_dropout(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    BMS_AppContext app;
    const uint32_t boot_ms = 3000U;

    Telemetry_Init();
    Telemetry_SetSink(0, 0);

    MockI2C_Reset(&mock);
    seed_healthy_devices(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    BMS_App_Init(&app);
    BMS_App_OnBoot(&app, boot_ms);
    expect_true("health_boot_idle", BMS_App_GetState(&app) == BMS_STATE_IDLE);

    /* Gauge disappears on next poll */
    MockI2C_Reset(&mock);
    seed_charger_only(&mock);
    BMS_App_Service(&app, boot_ms + BMS_POLL_INTERVAL_MS);
    expect_true("health_dropout_fault", BMS_App_GetState(&app) == BMS_STATE_FAULT);

    /* Two consecutive recoveries required */
    seed_healthy_devices(&mock);
    BMS_App_Service(&app, boot_ms + BMS_POLL_INTERVAL_MS + BMS_IC_RETRY_INTERVAL_MS);
    expect_true("health_recover_one", BMS_App_GetState(&app) == BMS_STATE_FAULT);
    BMS_App_Service(&app,
                    boot_ms + BMS_POLL_INTERVAL_MS + (2U * BMS_IC_RETRY_INTERVAL_MS));
    expect_true("health_recover_two", BMS_App_GetState(&app) == BMS_STATE_IDLE);
}

static void test_boot_retry_timing(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    BMS_AppContext app;
    uint32_t reads_after_boot;
    const uint32_t boot_ms = 4000U;

    Telemetry_Init();
    Telemetry_SetSink(0, 0);

    MockI2C_Reset(&mock);
    seed_healthy_devices(&mock);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    BMS_App_Init(&app);
    BMS_App_OnBoot(&app, boot_ms);
    reads_after_boot = mock.read_count;

    BMS_App_Service(&app, boot_ms + BMS_POLL_INTERVAL_MS - 1U);
    expect_true("timing_no_early_probe", mock.read_count == reads_after_boot);

    BMS_App_Service(&app, boot_ms + BMS_POLL_INTERVAL_MS);
    expect_true("timing_probes_after_interval", mock.read_count > reads_after_boot);
}

static void test_system_status_required(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    LTC4162_Identity ci;
    BMS_Status st;

    MockI2C_Reset(&mock);
    MockI2C_SetReg(&mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHEM_CELLS,
                   (uint16_t)((LTC4162_CHEM_FAD << 8) | 4U));
    MockI2C_SetReg(&mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHARGER_STATE, 256U);
    MockI2C_AddScript(&mock, BMS_I2C_BUS_CHARGER, BMS_LTC4162_ADDR_HAL,
                      LTC4162_REG_SYSTEM_STATUS, 0, MOCK_I2C_RESP_TIMEOUT, 0);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    ci.present = 1U;
    st = LTC4162_CheckDevice(&ci);
    expect_true("sys_status_timeout", st == BMS_ERR_TIMEOUT);
    expect_true("sys_status_no_mutate", ci.present == 1U);
}

static void test_wrong_chem_rejected(void)
{
    MockI2C_State mock;
    BMS_I2C_Transport t;
    LTC4162_Identity ci;
    BMS_Status st;

    MockI2C_Reset(&mock);
    /* Li-ion LAD chemistry = 0 — wrong for FADM board */
    MockI2C_SetReg(&mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHEM_CELLS, 0x0004U);
    MockI2C_SetReg(&mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_CHARGER_STATE, 256U);
    MockI2C_SetReg(&mock, BMS_LTC4162_ADDR_HAL, LTC4162_REG_SYSTEM_STATUS, 1U);
    t = MockI2C_MakeTransport(&mock);
    BMS_I2C_SetTransport(&t);

    ci.present = 1U;
    st = LTC4162_CheckDevice(&ci);
    expect_true("wrong_chem_fails", st == BMS_ERR_DEVICE);
    expect_true("wrong_chem_no_mutate", ci.present == 1U); /* unchanged on failure */
}

static void test_telemetry_queue_bounds(void)
{
    uint32_t i;
    uint32_t dropped_before;

    Telemetry_Init();
    Telemetry_SetSink(0, 0);
    dropped_before = Telemetry_DroppedCount();
    for (i = 0; i < (BMS_TELEMETRY_QUEUE_DEPTH + 5U); i++) {
        (void)Telemetry_LogEvent("STORM", "x");
    }
    expect_true("queue_drops", Telemetry_DroppedCount() > dropped_before);
    expect_true("queue_capped", Telemetry_QueuedCount() == BMS_TELEMETRY_QUEUE_DEPTH);
}

int main(void)
{
    g_failures = 0;
    printf("=== BMS Milestone 1 host tests ===\n");
    test_responding_devices();
    test_nack_timeout_bus();
    test_disabled_writes();
    test_thresholds_unconfigured();
    test_boot_and_two_cycle_recovery();
    test_recovery_resets_after_fail();
    test_idle_health_dropout();
    test_boot_retry_timing();
    test_system_status_required();
    test_wrong_chem_rejected();
    test_telemetry_queue_bounds();
    printf("=== %s (%d failures) ===\n",
           (g_failures == 0) ? "ALL PASSED" : "FAILED", g_failures);
    return (g_failures == 0) ? 0 : 1;
}
