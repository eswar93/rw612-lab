# 01 - Blinky (Zephyr)

The "hello world" of embedded: make an LED blink, but this time through Zephyr.

**Analogy:** the LED is a lamp, the device tree is the wiring diagram on the wall, and `main.c` is me flipping the switch once a second without needing to know which wire goes where.

## What it does

Toggles the board's `led0` once per second.

## Build

See [../../docs/setup-windows.md](../../docs/setup-windows.md).

## Notes to self

- Check that `led0` actually exists for `frdm_rw612` in the board's devicetree; if not, find the right alias.
- _Photo of the blinking board goes here later._
