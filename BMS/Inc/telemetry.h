/**
 * @file telemetry.h
 * @brief Bounded typed CSV logging (BOOT / DATA / EVENT) for UART or host sink.
 */
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stddef.h>
#include <stdint.h>

#include "bms_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Telemetry_SinkFn)(const char *line, size_t len, void *ctx);

void Telemetry_Init(void);
void Telemetry_SetSink(Telemetry_SinkFn sink, void *ctx);

/** Enqueue a formatted record. Drops and counts if the queue is full. */
BMS_Status Telemetry_LogBoot(const char *message);
BMS_Status Telemetry_LogEvent(const char *source, const char *detail);
BMS_Status Telemetry_LogI2cFailure(BMS_I2C_Bus bus, uint16_t dev_addr_hal,
                                   uint8_t reg, BMS_Status status);

/** Drain up to max_lines to the configured sink. Returns lines emitted. */
uint32_t Telemetry_Service(uint32_t max_lines);

uint32_t Telemetry_DroppedCount(void);
uint32_t Telemetry_QueuedCount(void);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_H */
