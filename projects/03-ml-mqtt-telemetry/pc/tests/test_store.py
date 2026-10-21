import unittest
from datetime import datetime, timedelta, timezone

from collector.messages import Reading
from collector.store import Store


def reading(seq, uptime_ms, temp_mc=24000, device="rw612-01"):
    return Reading(device=device, seq=seq, uptime_ms=uptime_ms, temp_mc=temp_mc)


class StoreTests(unittest.TestCase):
    def setUp(self):
        self.store = Store(":memory:")

    def tearDown(self):
        self.store.close()

    def test_add_and_count(self):
        self.assertTrue(self.store.add(reading(0, 1000)))
        self.assertTrue(self.store.add(reading(1, 2000)))
        self.assertEqual(self.store.count(), 2)

    def test_duplicate_is_ignored(self):
        self.assertTrue(self.store.add(reading(5, 6000)))
        self.assertFalse(self.store.add(reading(5, 6000)))
        self.assertEqual(self.store.count(), 1)

    def test_received_at_is_utc_iso(self):
        when = datetime(2026, 10, 21, 12, 0, 0, tzinfo=timezone(timedelta(hours=2)))
        self.store.add(reading(0, 1000), received_at=when)
        self.assertEqual(self.store.latest(1)[0][5], "2026-10-21T10:00:00.000+00:00")

    def test_gaps_in_seq_stay_in_the_same_boot(self):
        self.store.add(reading(0, 1000))
        self.store.add(reading(7, 8000))  # board was reconnecting: samples 1..6 missed
        boots = {row[1] for row in self.store.latest(10)}
        self.assertEqual(boots, {0})

    def test_reboot_starts_a_new_boot_id(self):
        self.store.add(reading(40, 41000))
        self.store.add(reading(41, 42000))
        self.store.add(reading(0, 1000))  # seq and uptime went backwards: the board rebooted
        self.store.add(reading(1, 2000))
        rows = list(reversed(self.store.latest(10)))
        self.assertEqual([row[1] for row in rows], [0, 0, 1, 1])

    def test_devices_are_independent(self):
        self.store.add(reading(10, 11000, device="a"))
        self.store.add(reading(0, 1000, device="b"))  # b starts at 0: not a reboot of a
        self.store.add(reading(11, 12000, device="a"))
        by_device = {row[0]: row for row in self.store.devices()}
        self.assertEqual(by_device["a"][4], 1)
        self.assertEqual(by_device["b"][4], 1)

    def test_data_survives_reopening(self):
        import os
        import tempfile

        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "sub", "t.db")
            first = Store(path)
            first.add(reading(0, 1000))
            first.close()
            second = Store(path)
            self.assertEqual(second.count(), 1)
            second.add(reading(1, 2000))
            self.assertEqual(second.latest(1)[0][1], 0)  # still the same boot after restart
            second.close()


if __name__ == "__main__":
    unittest.main()
