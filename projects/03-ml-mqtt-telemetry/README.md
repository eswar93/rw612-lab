# 03 - ML / MQTT telemetry (Zephyr, C++17)

**Status:** step M0 done (Wi-Fi works). Step M1 (sensor in C++) written, **not yet tested on my board** (update after the first run).

**Analogy:** the board is a weather station on my desk. It measures, sends reports over Wi-Fi (MQTT) to a mailbox on my PC, a clerk files them in a database, and a student (the model) studies the archive and learns what a normal day looks like, so it can raise a hand when something is off.

<!-- TODO: photo of the setup -->
<!-- TODO: flowchart: sensor -> MQTT -> broker -> database -> training -> model -> back to the board / alerts -->

## Plan

| Step | What | What I learn |
|---|---|---|
| M0 | Wi-Fi on the FRDM-RW612 with Zephyr's wifi shell sample | Zephyr networking, binary blobs |
| M1 | Read the P3T1755 with Zephyr's sensor API, in C++17 | Devicetree overlays, C++ in Zephyr |
| M2 | Connect to Wi-Fi from the app and publish readings over MQTT to a broker on my PC | MQTT topics and QoS, Zephyr's MQTT client |
| M3 | A Python service stores the readings in a database | Schema design, time series |
| M4 | Train an anomaly detector on the stored data (with CUDA) | The training workflow |
| M5 | Run the model: first on the PC (alerts back over MQTT), then maybe on the chip | Deployment, memory budgets |

One honest limitation: a single temperature sensor is a small dataset. It is enough to build and understand the whole pipeline, which is the point. Richer signals (Wi-Fi signal strength, a second sensor) can come later.

## Why C++17

Zephyr is C. I use C++17 for the application layer only, with a small set of rules:

- no exceptions, no RTTI (Zephyr leaves them off by default),
- no heap allocation in the sampling loop,
- no hardware access in global constructors (Zephyr constructs globals before `main()`),
- thin classes around Zephyr's C API instead of clever templates.

## Build and flash

```powershell
.\tools\zephyr-build.ps1 -Project 03-ml-mqtt-telemetry -Clean
.\tools\zephyr-build.ps1 -Project 03-ml-mqtt-telemetry -Flash
```

Open the serial console at 115200 baud. Expected:

```
Project 3 (C++17): reading the P3T1755 every 1000 ms
#0  Temperature: 24.375 C
```

## Notes from M0 (Wi-Fi)

- `west blobs fetch hal_nxp` is needed for Wi-Fi and Bluetooth.
- `wifi connect` needs the key-management type that matches the network (`-k`). Run `wifi scan` first and read the security column.
- Wi-Fi passwords never go into the repo. Local secrets will live in a git-ignored `secrets.conf`.
