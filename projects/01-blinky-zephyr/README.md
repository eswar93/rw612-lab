# 01 - Blinky (Zephyr)

The "hello world" of embedded: make an LED blink, but this time through Zephyr.

**Status:** working on my FRDM-RW612.

**Analogy:** the LED is a lamp, the device tree is the wiring diagram on the wall, and `main.c` is me flipping the switch once a second without needing to know which wire goes where.

<!-- TODO: photo of the blinking board -->
<!-- TODO: flowchart (boot -> init GPIO -> loop: toggle, sleep) -->

## What it does

Toggles the board's `led0` in a loop with `k_msleep()`. `SLEEP_TIME_MS` sets the delay.

## Build and flash

See [../../docs/setup-windows.md](../../docs/setup-windows.md), sections 5 and 6.

## What I learned

- In the board's devicetree, `led0` is an alias for `green_led`, on `hsgpio0` pin 12. That is the only LED Zephyr defines for this board.
- I could not tell from looking whether the LED was mine: the board already blinked its green LED before I flashed anything. Changing the delay to 100 ms made the difference obvious.
- The red and blue parts of the RGB LED have no Zephyr names. Using them would need a devicetree overlay and the board schematic. Maybe later.

## Open questions

- Which physical LED is GPIO0_12 on the schematic?
