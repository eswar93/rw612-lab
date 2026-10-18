# 02 - On-board temperature (FreeRTOS)

**Status:** planned.

Goal: read the on-board temperature sensor from a FreeRTOS task using NXP's SDK and print it over the serial console.

**Analogy:** a task is a worker who wakes up every second, checks the thermometer, and writes the number on a whiteboard (the UART).

Open questions:
- Which sensor is on the FRDM-RW612, and on which bus? (check the board user manual)
- Which SDK flow will I use for FreeRTOS: the VS Code extension or `west`?
