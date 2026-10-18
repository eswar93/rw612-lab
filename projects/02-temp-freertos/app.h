#ifndef APP_H_
#define APP_H_

/* P3T1755 temperature sensor on the FRDM-RW612 (board user manual UM12160, section 2.5):
 *  - FlexComm 2 in I2C mode, SDA = GPIO_16, SCL = GPIO_17
 *  - the manual prints the address as 0x90 (8-bit form); the 7-bit address the driver wants is 0x48 */
#define TEMP_I2C_BASE         I2C2
#define TEMP_I2C_CLOCK_HZ     CLOCK_GetFlexCommClkFreq(2U)
#define TEMP_SENSOR_ADDR_7BIT 0x48U

void BOARD_InitHardware(void);

#endif /* APP_H_ */
