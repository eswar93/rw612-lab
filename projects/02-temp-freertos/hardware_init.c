/*
 * Board bring-up for project 2.
 * Follows the structure of NXP's own hardware_init.c for the FRDM-RW612 I2C examples
 * (Copyright NXP, BSD-3-Clause), but sets up the pins directly so this file is self-contained.
 */
#include "clock_config.h"
#include "board.h"
#include "fsl_io_mux.h"

void BOARD_InitHardware(void)
{
    /* 16 MHz clock for FLEXCOMM2 (what NXP's I2C driver examples use) */
    CLOCK_AttachClk(kSFRO_to_FLEXCOMM2);

    /* FC3 USART on GPIO_24: the debug console, reaches the PC through the MCU-Link VCOM port */
    IO_MUX_SetPinMux(IO_MUX_FC3_USART_DATA);

    /* FC2 I2C on GPIO_16 (SDA) and GPIO_17 (SCL): the street where the P3T1755 lives */
    IO_MUX_SetPinMux(IO_MUX_FC2_I2C_16_17);

    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
}
