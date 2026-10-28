/**
 * @file telemetry.c
 * @brief Bounded CSV log queue. Failure storms drop records instead of blocking.
 */
#include "telemetry.h"

#include <stdio.h>
#include <string.h>

#include "bms_config.h"
#include "bms_i2c.h"

typedef struct {
    char line[BMS_TELEMETRY_LINE_MAX];
} TelemetryRecord;

static TelemetryRecord s_queue[BMS_TELEMETRY_QUEUE_DEPTH];
static uint32_t s_head;
static uint32_t s_tail;
static uint32_t s_count;
static uint32_t s_dropped;
static Telemetry_SinkFn s_sink;
static void *s_sink_ctx;

void Telemetry_Init(void)
{
    s_head = 0U;
    s_tail = 0U;
    s_count = 0U;
    s_dropped = 0U;
    s_sink = 0;
    s_sink_ctx = 0;
}

void Telemetry_SetSink(Telemetry_SinkFn sink, void *ctx)
{
    s_sink = sink;
    s_sink_ctx = ctx;
}

static BMS_Status enqueue_line(const char *line)
{
    size_t len;

    if (line == 0) {
        return BMS_ERR_INVALID_ARG;
    }
    len = strlen(line);
    if (len >= BMS_TELEMETRY_LINE_MAX) {
        len = BMS_TELEMETRY_LINE_MAX - 1U;
    }

    if (s_count >= BMS_TELEMETRY_QUEUE_DEPTH) {
        s_dropped++;
        return BMS_ERR_NOT_READY;
    }

    memcpy(s_queue[s_tail].line, line, len);
    s_queue[s_tail].line[len] = '\0';
    s_tail = (s_tail + 1U) % BMS_TELEMETRY_QUEUE_DEPTH;
    s_count++;
    return BMS_OK;
}

BMS_Status Telemetry_LogBoot(const char *message)
{
    char buf[BMS_TELEMETRY_LINE_MAX];
    int n;

    n = snprintf(buf, sizeof(buf), "BOOT,%u.%u.%u,%s",
                 BMS_FW_VERSION_MAJOR, BMS_FW_VERSION_MINOR, BMS_FW_VERSION_PATCH,
                 (message != 0) ? message : "");
    if (n < 0) {
        return BMS_ERR_INVALID;
    }
    return enqueue_line(buf);
}

BMS_Status Telemetry_LogEvent(const char *source, const char *detail)
{
    char buf[BMS_TELEMETRY_LINE_MAX];
    int n;

    n = snprintf(buf, sizeof(buf), "EVENT,%s,%s",
                 (source != 0) ? source : "?",
                 (detail != 0) ? detail : "");
    if (n < 0) {
        return BMS_ERR_INVALID;
    }
    return enqueue_line(buf);
}

BMS_Status Telemetry_LogI2cFailure(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                                   uint8_t reg, BMS_Status status)
{
    char buf[BMS_TELEMETRY_LINE_MAX];
    int n;

    n = snprintf(buf, sizeof(buf),
                 "EVENT,I2C_FAIL,%s,0x%02X,reg=0x%02X,status=%s",
                 BMS_I2C_BusName(bus), (unsigned)dev_addr_hal, (unsigned)reg,
                 BMS_StatusName(status));
    if (n < 0) {
        return BMS_ERR_INVALID;
    }
    return enqueue_line(buf);
}

uint32_t Telemetry_Service(uint32_t max_lines)
{
    uint32_t emitted = 0U;

    while ((emitted < max_lines) && (s_count > 0U)) {
        const char *line = s_queue[s_head].line;
        size_t len = strlen(line);
        if (s_sink != 0) {
            s_sink(line, len, s_sink_ctx);
        }
        s_head = (s_head + 1U) % BMS_TELEMETRY_QUEUE_DEPTH;
        s_count--;
        emitted++;
    }
    return emitted;
}

uint32_t Telemetry_DroppedCount(void)
{
    return s_dropped;
}

uint32_t Telemetry_QueuedCount(void)
{
    return s_count;
}
