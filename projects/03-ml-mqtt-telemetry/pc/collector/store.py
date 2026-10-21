"""SQLite storage for the readings.

One table, one row per reading. The only extra thing the collector adds is received_at:
the board has no real-time clock, so the PC stamps each message with UTC when it arrives.

boot_id counts reboots per device. A reboot is detected when seq or uptime_ms goes backwards,
so a model can later avoid building a time window across a reboot.
"""

import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

from .messages import Reading

SCHEMA = """
CREATE TABLE IF NOT EXISTS readings (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    device      TEXT    NOT NULL,
    boot_id     INTEGER NOT NULL,
    seq         INTEGER NOT NULL,
    uptime_ms   INTEGER NOT NULL,
    temp_mc     INTEGER NOT NULL,
    received_at TEXT    NOT NULL,           -- ISO 8601, UTC
    UNIQUE (device, boot_id, seq)
);
CREATE INDEX IF NOT EXISTS idx_readings_device_time ON readings (device, received_at);
"""


class Store:
    def __init__(self, path):
        self._path = str(path)
        if self._path != ":memory:":
            Path(self._path).parent.mkdir(parents=True, exist_ok=True)
        self._db = sqlite3.connect(self._path)
        self._db.executescript(SCHEMA)

    def close(self) -> None:
        self._db.close()

    def _boot_id_for(self, reading: Reading) -> int:
        row = self._db.execute(
            "SELECT boot_id, seq, uptime_ms FROM readings WHERE device = ? ORDER BY id DESC LIMIT 1",
            (reading.device,),
        ).fetchone()
        if row is None:
            return 0
        last_boot, last_seq, last_uptime = row
        if reading.seq < last_seq or reading.uptime_ms < last_uptime:
            return last_boot + 1
        return last_boot

    def add(self, reading: Reading, received_at: Optional[datetime] = None) -> bool:
        """Store one reading. Returns False if it was a duplicate and was ignored."""
        when = received_at or datetime.now(timezone.utc)
        cursor = self._db.execute(
            "INSERT OR IGNORE INTO readings (device, boot_id, seq, uptime_ms, temp_mc, received_at) "
            "VALUES (?, ?, ?, ?, ?, ?)",
            (
                reading.device,
                self._boot_id_for(reading),
                reading.seq,
                reading.uptime_ms,
                reading.temp_mc,
                when.astimezone(timezone.utc).isoformat(timespec="milliseconds"),
            ),
        )
        self._db.commit()
        return cursor.rowcount == 1

    def count(self) -> int:
        return self._db.execute("SELECT COUNT(*) FROM readings").fetchone()[0]

    def latest(self, n: int = 5):
        """The n newest rows as (device, boot_id, seq, uptime_ms, temp_mc, received_at)."""
        return self._db.execute(
            "SELECT device, boot_id, seq, uptime_ms, temp_mc, received_at "
            "FROM readings ORDER BY id DESC LIMIT ?",
            (n,),
        ).fetchall()

    def devices(self):
        """One row per device: (device, rows, first_received_at, last_received_at, boots)."""
        return self._db.execute(
            "SELECT device, COUNT(*), MIN(received_at), MAX(received_at), COUNT(DISTINCT boot_id) "
            "FROM readings GROUP BY device ORDER BY device"
        ).fetchall()
