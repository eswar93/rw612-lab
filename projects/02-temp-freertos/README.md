# 02 - On-board temperature (FreeRTOS)

**Status:** first version written, **not yet tested on my board** (update this line after the first run).

Two FreeRTOS tasks and a queue read the board's P3T1755 temperature sensor once per second and print the value on the serial console.

**Analogy:** a small café. The *sensor task* is the waiter who checks the thermometer and drops a note in the mailbox. The *queue* is the mailbox. The *print task* is the cashier who reads each note and announces it. Neither of them ever waits for the other's slow part.

<!-- TODO: photo of the board with the serial console output -->
<!-- TODO: flowchart: sensor_task -> I2C read -> queue -> print_task -> UART -->

## Facts about the hardware (from the FRDM-RW612 user manual, UM12160, section 2.5)

- The sensor is an NXP **P3T1755**, wired as plain **I2C** on **FlexComm 2**: SDA = GPIO_16, SCL = GPIO_17.
- The manual prints its address as **0x90** (8-bit form). The driver wants the **7-bit** address: **0x48**.
- The same two pins also reach the Arduino J2 header, the mikroBUS J5 socket and the Pmod J7 header. Keep that in mind before plugging in a shield.

## Build and flash

Setup is in [../../docs/setup-windows-nxp.md](../../docs/setup-windows-nxp.md). Then, from the repo root:

```powershell
.\tools\nxp-build.ps1 -Project 02-temp-freertos -Clean          # build
.\tools\nxp-build.ps1 -Project 02-temp-freertos -Flash          # build and flash
```

Open the board's serial port at 115200 baud. Expect something like:

```
FRDM-RW612 temperature demo (FreeRTOS, P3T1755 on FC2 I2C)
#0  Temperature: 24.375 C
#1  Temperature: 24.437 C
```

## What I still need to check

- [ ] Compare the readings with NXP's `temperature_measurement` demo and a room thermometer.
- [ ] Check the register conversion (12-bit, 0.0625 C per count) against the P3T1755 data sheet.
- [ ] The I2C read is a polling (blocking) call inside a task. That works, but the task spins while the bus is busy. Next step: try an interrupt-driven transfer.
- [ ] Add a CI job for this project.
