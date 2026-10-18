# CI notes (GitHub Actions)

Every push and every pull request builds **every project** from a clean machine and uploads the firmware as a downloadable artifact. If a project's badge in the main README is green, it really builds without anything hidden on my PC.

**Analogy:** GitHub Actions is a rented kitchen with no dining room. It cooks the firmware from scratch every time, but it cannot serve it: the board is on my desk, so flashing stays a manual step.

## One workflow per project

| Project | Workflow file | Artifact name |
|---|---|---|
| 1 - blinky (Zephyr) | `.github/workflows/project-01-blinky-zephyr.yml` | `blinky-zephyr-frdm_rw612` |
| 2 - temperature (FreeRTOS) | `.github/workflows/project-02-temp-freertos.yml` | `temp-freertos-frdmrw612` |

One file per project, so each project gets its own badge in the main README. A new project means one new workflow file and one new row in the README table.

## Project 1: Zephyr blinky

1. **Check out this repo** into a folder called `app`.
2. **Install Python, create a virtual environment, and install a few system tools** (Ninja, gperf, dtc).
3. **Get Zephyr at one pinned commit.** It is the same commit I built with on my PC (`ZEPHYR_REF`), so CI and my machine cook the same dish. Then `west update` fetches the matching modules.
4. **Install the Zephyr SDK**, ARM toolchain only, to keep it quick.
5. **Build** `projects/01-blinky-zephyr` for `frdm_rw612`.
6. **Upload** `zephyr.bin`, `.elf` and `.hex`.

When I update Zephyr locally: run `git -C C:\dev\zephyrproject\zephyr describe --tags` and copy the commit part (after the `g`) into `ZEPHYR_REF`.

## Project 2: NXP SDK + FreeRTOS

A different kitchen: NXP's SDK has its own recipe books and toolbox, so this job is set up differently.

1. **Check out this repo**, set up Python and a virtual environment, install `west`, `cmake`, `ninja` and `jsonschema` with pip.
2. **Download the Arm GNU toolchain** (`ARM_GCC_VERSION`, same version as on my PC) and point `ARMGCC_DIR` at the folder above `bin`.
3. **Get the NXP SDK at one pinned tag** (`SDK_TAG`) with `west init`, then `west update_board --set board frdmrw612` so only this board's repositories are downloaded, not the whole 7-8 GB SDK. The result is **cached**, so later runs skip the download.
4. **Install the SDK's own Python requirements**, if the SDK ships a `requirements.txt`.
5. **Register the SDK** with `west mcuxsdk-export`, so `find_package(McuxSDK)` in the project's `CMakeLists.txt` can find it.
6. **Build** `projects/02-temp-freertos` for `frdmrw612`.
7. **Upload** `app.elf`, `app.hex`, `app.bin` and `output.map`.

When I change SDK version locally: update `SDK_TAG`, and change the `-v1` at the end of the cache key to `-v2` so CI downloads the new SDK instead of using the old cache.

## What it does not do

- It does not flash the board. That would need a self-hosted runner with the board plugged in. Not planning that.
- It does not run the code. Both projects need real hardware.

## Running it by hand

Actions tab -> pick a workflow -> **Run workflow**.

## Problems I hit

- **Project 1, first run: `No module named 'jsonschema'`.** Same mistake as on my PC: `west zephyr-export` ran before `west packages pip --install`. Swapped the order.
- **Project 1, second failure: `Running pip install outside of a virtual environment`.** Zephyr's `west packages pip --install` refuses to install into the system Python, same idea as on my PC. Added a venv step; its `bin` folder goes on `PATH` through `$GITHUB_PATH` so the later steps use it.
- **Project 2:** _(first run not done yet)_
