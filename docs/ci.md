# CI notes (GitHub Actions)

Every push builds the Zephyr blinky from a clean machine. If it goes green, the repo really builds without anything hidden on my PC.

**Analogy:** GitHub Actions is a rented kitchen with no dining room. It cooks the firmware from scratch every time, but it cannot serve it: the board is on my desk, so flashing stays a manual step.

## What the workflow does

File: `.github/workflows/build.yml`

1. **Check out this repo** into a folder called `app`.
2. **Install Python, create a virtual environment, and install a few system tools** (Ninja, gperf, dtc).
3. **Get Zephyr at one pinned commit.** It is the same commit I built with on my PC (`ZEPHYR_REF`), so CI and my machine cook the same dish. Then `west update` fetches the matching modules.
4. **Install the Zephyr SDK**, ARM toolchain only, to keep it quick.
5. **Build** `projects/01-blinky-zephyr` for `frdm_rw612`.
6. **Upload** `zephyr.bin`, `.elf` and `.hex` as a downloadable artifact.

## What it does not do

- It does not flash the board. That would need a self-hosted runner with the board plugged in. Not planning that.
- It does not run the code. Blinky needs real hardware.

## When I update Zephyr locally

Run `git -C C:\dev\zephyrproject\zephyr describe --tags` and copy the commit part (after the `g`) into `ZEPHYR_REF` in the workflow.

## Running it by hand

Actions tab -> **build** -> **Run workflow**.

## Problems I hit

- **First run failed with `No module named 'jsonschema'`.** Same mistake as on my PC: `west zephyr-export` ran before `west packages pip --install`. Swapped the order.
- **Second failure: `Running pip install outside of a virtual environment`.** Zephyr's `west packages pip --install` refuses to install into the system Python, same idea as on my PC. Added a venv step; its `bin` folder goes on `PATH` through `$GITHUB_PATH` so the later steps use it.
