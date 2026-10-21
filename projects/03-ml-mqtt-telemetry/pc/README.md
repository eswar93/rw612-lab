# PC side of project 3: the collector

**Analogy:** the clerk at the post office. It stands at the mailbox `rw612/telemetry`, checks each postcard, stamps it with the arrival time, and files it in a cabinet (SQLite) so the student (the model, later) can study the archive.

## What it does

- subscribes to the board's MQTT topic,
- checks every message (valid JSON, whole numbers, plausible temperature) and rejects bad ones with a warning,
- stamps each reading with the PC's UTC time, because the board has no real-time clock,
- detects reboots (when `seq` or `uptime_ms` goes backwards) and numbers them with `boot_id`, so a model never builds a time window across a reboot,
- ignores duplicates.

## Setup (once)

A separate virtual environment for the PC side, so it does not mix with the Zephyr one:

```powershell
cd projects\03-ml-mqtt-telemetry\pc
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

## Run

Mosquitto must be running (see [../../../docs/mqtt-broker-windows.md](../../../docs/mqtt-broker-windows.md)) and the board powered on.

```powershell
python -m collector --broker localhost
```

Expected: one line per second, like `rw612-01 #42  24.375 C`. Stop with Ctrl+C, which prints a count of stored, duplicate and rejected messages.

What is in the database?

```powershell
python -m collector.summary
```

The database file is `pc\data\telemetry.db` (git-ignored).

## Tests

```powershell
python -m unittest discover -s tests -v
```

No broker and no board needed. CI runs the same command.

## The table

| Column | Meaning |
|---|---|
| `device` | which board (`CONFIG_APP_DEVICE_ID`) |
| `boot_id` | 0 for the first boot seen, +1 after every detected reboot |
| `seq` | loop counter on the board; a jump means missed samples |
| `uptime_ms` | milliseconds since the board booted |
| `temp_mc` | thousandths of a degree Celsius |
| `received_at` | UTC time the PC received it (ISO 8601) |

## Why SQLite first

It needs no server, so the pipeline works end to end in one sitting. The database code is isolated in `collector/store.py`, so moving to PostgreSQL later (a good DB exercise) means rewriting one file, not the collector.
