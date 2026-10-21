"""Run with:  python -m collector.summary   (what is in the database?)"""

import argparse

from .__main__ import DEFAULT_DB
from .store import Store


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Show what the collector has stored")
    parser.add_argument("--db", default=str(DEFAULT_DB))
    parser.add_argument("-n", type=int, default=5, help="how many latest rows to show")
    args = parser.parse_args(argv)

    store = Store(args.db)
    print(f"Rows: {store.count()}")
    for device, rows, first, last, boots in store.devices():
        print(f"  {device}: {rows} rows, {boots} boot(s), {first} .. {last}")
    print(f"Latest {args.n}:")
    for device, boot_id, seq, uptime_ms, temp_mc, received_at in store.latest(args.n):
        print(f"  {received_at}  {device} boot {boot_id} #{seq}  {temp_mc / 1000:.3f} C")
    store.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
