# Windows 11 setup notes

These are the steps that actually worked for me on Windows 11 with PowerShell, ending with a blinking LED on the FRDM-RW612. Last checked: October 2026.
Zephyr's install steps change between releases, so if something here fails, compare with the official
[Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

## The mental model

Zephyr is a **kitchen**:

- `west` is the head chef with the shopping list. It fetches Zephyr and its modules.
- The **Zephyr SDK** is the knife set (the ARM compiler toolchain).
- **CMake + Ninja** are the recipe and the stove.
- **J-Link** is the waiter that carries the finished firmware to the board.

Stock the kitchen once, then each project is just "cook and serve".

## 1. Install the basic tools (once)

In a terminal:

```powershell
winget install Kitware.CMake Ninja-build.Ninja oss-winget.gperf Python.Python.3.12 Git.Git oss-winget.dtc wget 7zip.7zip
```

Close and reopen the terminal afterwards so the new tools are on PATH.

Also install the **SEGGER J-Link software**. The FRDM-RW612's on-board debug probe uses J-Link firmware by default, and `west flash` uses it (it reported J-Link 9.86a on my machine).

## 2. Build the Zephyr workspace (once)

The workspace lives **outside** this repo:

```
C:\dev\
├── zephyrproject\    # created by west, big, not committed
└── <this repo>\
```

```powershell
cd C:\dev
python -m venv zephyrproject\.venv
.\zephyrproject\.venv\Scripts\Activate.ps1
pip install west
west init zephyrproject
cd zephyrproject
west update
west packages pip --install
west zephyr-export
```

Order matters: `west zephyr-export` imports Python packages that `west packages pip --install` provides, so install them first.

If PowerShell refuses to run the activate script, run this once and retry:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

## 3. Install the Zephyr SDK (once)

```powershell
cd zephyr
west sdk install
```

## 4. Every new terminal

A new PowerShell window starts **without** the virtual environment. Before any `west` command:

```powershell
C:\dev\zephyrproject\.venv\Scripts\Activate.ps1
cd C:\dev\zephyrproject\zephyr
```

The prompt should start with `(.venv)`.

## 5. Build and flash project 1

Set the app path once per terminal (change it to wherever you cloned this repo):

```powershell
$app = "<path-to-this-repo>\projects\01-blinky-zephyr"
west build -b frdm_rw612 $app -d "$app\build" -p auto
west flash -d "$app\build"
```

Use `west -v flash ...` if you want more detail from the flashing step.

## 6. How to tell it really worked

The board may ship with a demo that already blinks, so "the LED blinks" is not proof.
Change `SLEEP_TIME_MS` in `src/main.c` to something very different (for example `100`), rebuild, flash, and see whether the blink rate changes.

## Problems I hit

| Symptom | Cause | Fix |
|---|---|---|
| `ModuleNotFoundError: No module named 'jsonschema'` on `west zephyr-export` | The venv was not active, so west used my global Python | Activate the venv with the PowerShell script (`Activate.ps1`, not `activate.bat`), then run `west packages pip --install` |
| `ModuleNotFoundError: No module named 'jsonschema'` (same error, second time, in CI) | I ran `west zephyr-export` before installing Zephyr's Python packages | Run `west packages pip --install` first, then `west zephyr-export` |
| The LED looked unchanged after flashing | The board already blinked the same LED with its factory firmware | Change the blink rate to something obviously different |

## Not needed yet

- `west blobs fetch hal_nxp` is only needed for Bluetooth or Wi-Fi, not for blinky.
