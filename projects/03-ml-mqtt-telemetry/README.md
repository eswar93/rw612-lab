# 03 - ML / MQTT telemetry (Zephyr, C++17)

**Status:** M0 (Wi-Fi) and M1 (sensor in C++) work. M2 (Wi-Fi + MQTT publish) is written, **not yet tested on my board** (update after the first run).

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

One honest limitation: a single temperature sensor is a small dataset. It is enough to build and understand the whole pipeline, which is the point.

## Why C++17

Zephyr is C. I use C++17 for the application layer only, with a small set of rules:

- no exceptions, no RTTI (Zephyr leaves them off by default),
- no heap allocation in the sampling loop,
- no hardware access in global constructors (Zephyr constructs globals before `main()`),
- thin classes around Zephyr's C API instead of clever templates.

## How the code is organised

| File | Job |
|---|---|
| `src/temp_sensor.*` | `TempSensor`: reads the P3T1755 through Zephyr's sensor API |
| `src/wifi_link.*` | `WifiLink`: connects to Wi-Fi and waits for a DHCP address |
| `src/mqtt_publisher.*` | `MqttPublisher`: MQTT 3.1.1 client, QoS 0, keep-alive |
| `src/main.cpp` | The loop: make sure the network is up, read, print, publish |
| `Kconfig` | My own settings (SSID, broker address, topic, device ID) |
| `boards/frdm_rw612.*` | Sensor overlay and the board settings copied from Zephyr's wifi sample |

The message on the topic `rw612/telemetry` looks like:

```json
{"device":"rw612-01","seq":42,"uptime_ms":42137,"temp_mC":24375}
```

Whole numbers only: millidegrees keep the message exact and the board never needs floating-point printing.

## Setup

1. The broker: [../../docs/mqtt-broker-windows.md](../../docs/mqtt-broker-windows.md).
2. Copy `secrets.conf.example` to `secrets.conf` in this folder and fill in the Wi-Fi name, password, security type (the `-k` number that worked in the wifi shell) and the PC's LAN address. `secrets.conf` is git-ignored. The firmware built with it contains the Wi-Fi password, so never share that binary. CI builds without it, so the CI artifact has no credentials and only prints locally.
3. `west blobs fetch hal_nxp` once (Wi-Fi firmware).

## Build and flash

```powershell
.\tools\zephyr-build.ps1 -Project 03-ml-mqtt-telemetry -Clean -Flash
```

The script passes `secrets.conf` automatically when it exists. In another terminal, watch the broker:

```powershell
cd "C:\Program Files\mosquitto"
.\mosquitto_sub.exe -h localhost -t "rw612/#" -v
```

Expected on the board's serial console (115200 baud):

```
Connecting to Wi-Fi "..." ...
Wi-Fi is up
Connecting to MQTT broker 192.168.x.y:1883 ...
MQTT connected
#0  Temperature: 24.375 C
```

and one JSON line per second in the broker terminal.

## Known limits of this step

- Everything runs in `main()`: while the board is reconnecting to Wi-Fi, no readings are taken. A separate network thread would fix that.
- QoS 0 only (fire and forget). QoS 1 with acknowledgements is a good next experiment.
- No TLS and no username/password yet. Home LAN only.
- No real-time clock: messages carry the uptime, and the PC adds the arrival time.

## Notes from M0 (Wi-Fi)

- `west blobs fetch hal_nxp` is needed for Wi-Fi and Bluetooth.
- `wifi connect` needs the key-management type that matches the network (`-k`). Run `wifi scan` first and read the security column.
