/*
 * Project 2: read the on-board P3T1755 temperature sensor from FreeRTOS tasks.
 *
 * The café analogy:
 *   sensor_task = the waiter: every second checks the thermometer, drops a note in the mailbox
 *   sample_queue = the mailbox between the two tasks
 *   print_task  = the cashier: reads each note and announces it on the serial console
 */
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "board.h"
#include "app.h"

#define TEMP_REG_TEMPERATURE 0x00U /* P3T1755 pointer register value that selects the temperature register */
#define SENSOR_PERIOD_MS     1000U
#define QUEUE_LENGTH         8U

typedef struct
{
    status_t status;       /* kStatus_Success, or the I2C driver error */
    int32_t milli_celsius; /* temperature in thousandths of a degree C */
} temp_sample_t;

static QueueHandle_t sample_queue = NULL;

static void sensor_task(void *pvParameters);
static void print_task(void *pvParameters);

/* Read the 16-bit temperature register: 12-bit two's complement, left-aligned, 0.0625 C per count.
 * NOTE: conversion written from the P3T1755 data sheet description; compare against a known
 * temperature (and NXP's temperature_measurement demo) before trusting it. */
static status_t temp_sensor_read(int32_t *milli_celsius)
{
    uint8_t raw[2]           = {0U, 0U};
    i2c_master_transfer_t xfer = {0};

    xfer.slaveAddress   = TEMP_SENSOR_ADDR_7BIT;
    xfer.direction      = kI2C_Read;
    xfer.subaddress     = TEMP_REG_TEMPERATURE;
    xfer.subaddressSize = 1U;
    xfer.data           = raw;
    xfer.dataSize       = sizeof(raw);
    xfer.flags          = kI2C_TransferDefaultFlag;

    status_t status = I2C_MasterTransferBlocking(TEMP_I2C_BASE, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    int16_t reg    = (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    int32_t counts = reg >> 4; /* arithmetic shift keeps the sign */
    *milli_celsius = (counts * 625) / 10;
    return kStatus_Success;
}

int main(void)
{
    i2c_master_config_t i2c_config;

    BOARD_InitHardware();
    PRINTF("\r\nFRDM-RW612 temperature demo (FreeRTOS, P3T1755 on FC2 I2C)\r\n");

    /* Default I2C master settings (standard mode, 100 kHz) are fine for a thermometer */
    I2C_MasterGetDefaultConfig(&i2c_config);
    I2C_MasterInit(TEMP_I2C_BASE, &i2c_config, TEMP_I2C_CLOCK_HZ);
    PRINTF("I2C ready\r\n");

    sample_queue = xQueueCreate(QUEUE_LENGTH, sizeof(temp_sample_t));
    if (sample_queue == NULL)
    {
        PRINTF("Queue creation failed!\r\n");
        for (;;)
            ;
    }

    if (xTaskCreate(sensor_task, "sensor", configMINIMAL_STACK_SIZE + 200, NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
    {
        PRINTF("Sensor task creation failed!\r\n");
        for (;;)
            ;
    }
    if (xTaskCreate(print_task, "print", configMINIMAL_STACK_SIZE + 200, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("Print task creation failed!\r\n");
        for (;;)
            ;
    }

    vTaskStartScheduler();

    for (;;)
        ;
}

static void sensor_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        temp_sample_t sample;

        sample.status = temp_sensor_read(&sample.milli_celsius);
        if (sample.status != kStatus_Success)
        {
            sample.milli_celsius = 0;
        }
        /* Do not block if the mailbox is full: dropping one reading beats stalling the sensor loop */
        (void)xQueueSend(sample_queue, &sample, 0);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

static void print_task(void *pvParameters)
{
    uint32_t count = 0U;

    for (;;)
    {
        temp_sample_t sample;

        if (xQueueReceive(sample_queue, &sample, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        if (sample.status != kStatus_Success)
        {
            PRINTF("#%u  I2C read failed, status %d\r\n", (unsigned int)count, (int)sample.status);
        }
        else
        {
            int32_t m          = sample.milli_celsius;
            const char *sign   = (m < 0) ? "-" : "";
            uint32_t magnitude = (uint32_t)((m < 0) ? -m : m);

            PRINTF("#%u  Temperature: %s%u.%03u C\r\n", (unsigned int)count, sign, (unsigned int)(magnitude / 1000U),
                   (unsigned int)(magnitude % 1000U));
        }
        count++;
    }
}
