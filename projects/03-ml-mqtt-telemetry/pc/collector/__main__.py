"""Run with:  python -m collector --broker localhost

Subscribes to the board's MQTT topic and stores every valid reading in SQLite.
"""

import argparse
import logging
from pathlib import Path

from .messages import MessageError, parse_message
from .store import Store

log = logging.getLogger("collector")

DEFAULT_DB = Path(__file__).resolve().parent.parent / "data" / "telemetry.db"


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Store the RW612's MQTT telemetry in SQLite")
    parser.add_argument("--broker", default="localhost", help="MQTT broker address (default: localhost)")
    parser.add_argument("--port", type=int, default=1883)
    parser.add_argument("--topic", default="rw612/telemetry")
    parser.add_argument("--db", default=str(DEFAULT_DB), help="SQLite file (default: pc/data/telemetry.db)")
    parser.add_argument("--client-id", default="rw612-collector")
    return parser.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")

    # Imported here so the tests and --help work without paho-mqtt installed
    import paho.mqtt.client as mqtt

    store = Store(args.db)
    counts = {"stored": 0, "duplicate": 0, "rejected": 0}

    def on_connect(client, userdata, connect_flags, reason_code, properties):
        if reason_code.is_failure:
            log.error("Broker refused the connection: %s", reason_code)
            return
        log.info("Connected to %s:%d, subscribing to %s", args.broker, args.port, args.topic)
        # Subscribe here, not once at start-up: paho reconnects by itself but does not resubscribe
        client.subscribe(args.topic, qos=0)

    def on_message(client, userdata, msg):
        try:
            reading = parse_message(msg.payload)
        except MessageError as exc:
            counts["rejected"] += 1
            log.warning("Rejected message on %s: %s (payload %r)", msg.topic, exc, msg.payload[:80])
            return

        if store.add(reading):
            counts["stored"] += 1
            log.info("%s #%d  %.3f C", reading.device, reading.seq, reading.temp_c)
        else:
            counts["duplicate"] += 1
            log.info("Ignored duplicate %s #%d", reading.device, reading.seq)

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=args.client_id)
    client.on_connect = on_connect
    client.on_message = on_message

    log.info("Database: %s", args.db)
    client.connect(args.broker, args.port, keepalive=60)
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        log.info("Stopping")
    finally:
        client.disconnect()
        store.close()
        log.info("Stored %(stored)d, duplicates %(duplicate)d, rejected %(rejected)d", counts)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
