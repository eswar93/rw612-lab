# Windows 11 setup notes: NXP MCUXpresso SDK (project 2)

Written from what actually happened on my machine (October 2026, SDK tag `v25.12.00`).
Items marked **(not yet verified)** are steps I have not run successfully myself yet.

## The mental model

Zephyr was a kitchen. NXP's SDK is a *second, different kitchen* that uses the same chef (`west`) but its own recipe books and its own toolbox:

- toolbox: the standard Arm GNU toolchain (`arm-none-eabi-gcc`), **not** the Zephyr SDK's `arm-zephyr-eabi`
- recipe books: the MCUXpresso SDK, cloned with `west`
- board names: NXP's names, which are **not** Zephyr's (`frdmrw612`, not `frdm_rw612`)

## 1. Arm GNU toolchain

```powershell
winget install -e --id Arm.GnuArmEmbeddedToolchain
```

Open a **new** terminal, then check: `where.exe arm-none-eabi-gcc`.

Tell NXP's build where it is. `ARMGCC_DIR` is the folder **above** `bin`, with forward slashes. Make it permanent:

```powershell
[Environment]::SetEnvironmentVariable("ARMGCC_DIR", "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1", "User")
```

## 2. SEGGER J-Link on PATH

The board's debug probe runs J-Link firmware, so `west flash -r jlink` needs the J-Link tools on PATH. Permanent version (adjust the folder name to your J-Link version):

```powershell
$old = [Environment]::GetEnvironmentVariable("Path","User")
[Environment]::SetEnvironmentVariable("Path", "$old;C:\Program Files\SEGGER\JLink_V986a", "User")
```

## 3. The SDK workspace (once)

```powershell
C:\dev\zephyrproject\.venv\Scripts\Activate.ps1     # reuse the Zephyr venv for west
cd C:\dev
west init -m https://github.com/nxp-mcuxpresso/mcuxsdk-manifests.git mcuxpresso-sdk --mr v25.12.00
cd mcuxpresso-sdk
west update
```

Plain `west update` downloaded a lot. A tutorial on NXP's SDK recommends `west update_board --set board <name>` to fetch only what one board needs. I have not tried that.

## 4. Sanity check with NXP's own demo (verified)

```powershell
cd C:\dev\mcuxpresso-sdk\mcuxsdk
west build -b frdmrw612 .\examples\demo_apps\temperature_measurement
west flash -r jlink
```

Serial console at 115200 baud: press any key and the demo prints a temperature.

## 5. Project 2 (freestanding app in this repo)

Project 2 lives in this repo, not inside the SDK. NXP's docs call that a freestanding application and use `find_package(McuxSDK)`, which needs the SDK exported once **(not yet verified)**:

```powershell
cd C:\dev\mcuxpresso-sdk\mcuxsdk
west mcuxsdk-export
```

Then from the repo root:

```powershell
.\tools\nxp-build.ps1 -Project 02-temp-freertos -Clean
.\tools\nxp-build.ps1 -Project 02-temp-freertos -Flash
```

If `west build` complains that the build directory belongs to another application, add `-nsc` (skip sanity check) to the `west build` line in the script. NXP's integration doc mentions this for projects not located inside the SDK.

## Problems I hit

| Symptom | Cause | Fix |
|---|---|---|
| `Please set ARMGCC_DIR in environment variables` | NXP's build looks for `arm-none-eabi`, which the Zephyr SDK does not provide | Install the Arm GNU toolchain and set `ARMGCC_DIR` |
| `Pristine build failed ... BINARY_DIR=None` | I used `-p always` on a build folder left half-finished by a failed first configure | Delete the `build` folder and run again without `-p` |
| `There is no board variable.cmake file ... frdm_rw612` | Zephyr's board name is not NXP's | Use `frdmrw612` |
| `ARMGCC_DIR` set but still not found | I had pointed it at the `bin` folder | Point it at the folder above `bin` |
| `i3c ... master_read_sensor_p3t1755` fails to build | That example has no FRDM-RW612 port. The board wires the sensor as I2C on FC2, not I3C | Use the I2C driver (project 2) |
| `FreeRTOS.h: No such file or directory` | A freestanding app does not inherit NXP's example-level FreeRTOS settings. My first `prj.conf` only opened the `cm33_non_trustzone` variant | Configure NXP's `freertos_queue` with `--cmake-only` into a scratch folder, compare its `.config` with mine, copy the missing `CONFIG_..freertos..` lines into `prj.conf` (kernel, heap_4, generated config, extension, config, and a few `configXXX` switches) |
| `mcux_convert_binary ... McuxSDK is library type`, then `west flash` says no `app.hex` or `app.bin` | That helper only works for repository-style examples | `CMakeLists.txt` creates `app.hex` and `app.bin` itself with `objcopy` after linking |
| `.\tools\nxp-build.ps1 cannot be loaded because running scripts is disabled` | PowerShell execution policy | `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned` once (the VS Code tasks bypass it) |
| Project 2 builds, links and flashes, but the console is silent and GDB shows the PC in RAM at an unknown address | The flash image had no flash configuration block (`.flash_config` at `0x08000000` was reserved but empty), so the boot ROM could not start it | Compile NXP's `examples/_boards/<board>/flash_config/flash_config.c` into the **executable** with `target_sources(app ...)`. In a static library the linker drops it because nothing references it. To spot this: compare `objdump -h` of a working example and mine, and check the first bytes of `app.bin` |
| `where.exe arm-none-eabi-gcc` found nothing right after install | The terminal was opened before the install | Open a new terminal |
