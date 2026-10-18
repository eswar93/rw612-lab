# rw612-lab

My hobby lab for learning embedded firmware on the **NXP FRDM-RW612**, one small project at a time.

> **Honest disclaimer:** I have ~11 years in embedded, but I'm new to this chip, to Zephyr, and to most of the AI/IoT side of things.
> This repo is my notebook. Expect wrong turns, notes-to-self, and things I fix later.

<!-- TODO: add a real photo of my FRDM-RW612 on the desk here -->
<!-- ![My FRDM-RW612](docs/images/devkit.jpg) -->

## Why I'm doing this

- Get comfortable with the RW612 (Cortex-M33 with Wi-Fi 6, BLE and 802.15.4 on one chip).
- Build the **same kind of thing on two RTOSes** (Zephyr and FreeRTOS) so I actually feel the differences instead of reading about them.
- Work toward AI-based IoT and safety ideas: sensor data goes out, a model gets trained, and something smarter comes back.
- Have real, runnable projects on GitHub instead of half-finished folders on my disk.

## My mental model (so I don't get lost)

Think of the RW612 as **one small workshop**. An RTOS is the *shop foreman* who decides which job gets the bench and when.

- **FreeRTOS** is a minimalist foreman: scheduler, queues, semaphores, and that's about it. I bring everything else.
- **Zephyr** is a foreman with a full filing system: device tree describes the hardware, Kconfig switches features on or off, and drivers are ready-made.

Same workshop, two management styles. That's the experiment.

## The projects

| # | Project | RTOS | Status | What I want to learn |
|---|---------|------|--------|----------------------|
| 1 | [Blink LED](projects/01-blinky-zephyr) | Zephyr | ✅ working | The Zephyr workflow end to end: build, flash, device tree, Kconfig |
| 2 | [Read on-board temperature](projects/02-temp-freertos) | FreeRTOS | 📝 planned | NXP SDK + FreeRTOS tasks, reading an I2C/I3C sensor |
| 3 | [ML / MQTT telemetry](projects/03-ml-mqtt-telemetry) | TBD | 💭 idea | MQTT, storing telemetry in a DB, training a model (maybe with CUDA), running or serving it |

Project 3 is deliberately vague for now. I'll shape it after 1 and 2 teach me what the board is good at.

## Repo layout

```
rw612-lab/
├── README.md
├── common/                      # RTOS-agnostic C code I want to reuse
├── projects/
│   ├── 01-blinky-zephyr/
│   ├── 02-temp-freertos/
│   └── 03-ml-mqtt-telemetry/
├── tools/                       # helper scripts
└── docs/
    ├── setup-windows.md         # how I set up my machine
    ├── images/                  # photos and screenshots
    └── diagrams/                # flowcharts
```

## My setup

- **Board:** NXP FRDM-RW612
- **Host:** Windows 11
- **Editor:** VS Code
- **Build:** CMake + Ninja, driven by `west` for Zephyr

Step by step notes are in [docs/setup-windows.md](docs/setup-windows.md).

## How the firmware flows

<!-- TODO: add flowchart for project 1 (boot -> init GPIO -> loop: toggle, sleep) -->
<!-- TODO: add flowchart for overall telemetry idea (board -> MQTT broker -> DB -> training -> model -> board) -->

*Diagrams coming once I understand things well enough to draw them.*

## Things I got wrong (running list)

- **Wrong terminal, wrong activate script.** I ran `activate.bat` while in PowerShell, so `west` used my global Python and could not find `jsonschema`. In PowerShell it is `Activate.ps1`.
- **Trusting an LED that was already blinking.** The board blinked green before I flashed anything, so I could not tell whether my firmware was running. Changing the delay to something very different proved it.

## Useful links I keep going back to

- Zephyr docs: https://docs.zephyrproject.org
- NXP FRDM-RW612 board page (search "FRDM-RW612" on nxp.com)

## License

MIT, so feel free to borrow anything. If you spot a mistake, please open an issue. I'd genuinely like to learn from it.
