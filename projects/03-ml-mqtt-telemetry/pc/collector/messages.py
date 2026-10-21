"""Turns the raw MQTT payload from the board into a checked Reading.

The board sends one JSON object per second, for example:
    {"device":"rw612-01","seq":42,"uptime_ms":42137,"temp_mC":24375}

Whole numbers only: temp_mC is thousandths of a degree Celsius.
"""

import json
from dataclasses import dataclass

# Plausibility limits for a room-temperature sensor, to catch garbage. Not a sensor spec.
MIN_TEMP_MC = -50_000
MAX_TEMP_MC = 150_000


class MessageError(ValueError):
    """The payload is not a valid telemetry message."""


@dataclass(frozen=True)
class Reading:
    device: str
    seq: int          # counts loop iterations on the board; gaps mean missed samples
    uptime_ms: int    # since the board booted; restarts from 0 after a reboot
    temp_mc: int      # thousandths of a degree Celsius

    @property
    def temp_c(self) -> float:
        return self.temp_mc / 1000.0


def _require_int(data: dict, key: str) -> int:
    value = data.get(key)
    # bool is a subclass of int in Python, but true/false is not a number here
    if isinstance(value, bool) or not isinstance(value, int):
        raise MessageError(f"'{key}' must be a whole number, got {value!r}")
    return value


def parse_message(payload: bytes) -> Reading:
    try:
        data = json.loads(payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise MessageError(f"not valid JSON: {exc}") from exc

    if not isinstance(data, dict):
        raise MessageError("payload must be a JSON object")

    device = data.get("device")
    if not isinstance(device, str) or not device:
        raise MessageError("'device' must be a non-empty string")

    seq = _require_int(data, "seq")
    uptime_ms = _require_int(data, "uptime_ms")
    temp_mc = _require_int(data, "temp_mC")

    if seq < 0 or uptime_ms < 0:
        raise MessageError("'seq' and 'uptime_ms' must not be negative")
    if not MIN_TEMP_MC <= temp_mc <= MAX_TEMP_MC:
        raise MessageError(f"temperature {temp_mc} mC is outside the plausible range")

    return Reading(device=device, seq=seq, uptime_ms=uptime_ms, temp_mc=temp_mc)
