import unittest

from collector.messages import MessageError, parse_message


class ParseMessageTests(unittest.TestCase):
    def test_valid_message(self):
        r = parse_message(b'{"device":"rw612-01","seq":42,"uptime_ms":42137,"temp_mC":24375}')
        self.assertEqual((r.device, r.seq, r.uptime_ms, r.temp_mc), ("rw612-01", 42, 42137, 24375))
        self.assertAlmostEqual(r.temp_c, 24.375)

    def test_negative_temperature_is_fine(self):
        r = parse_message(b'{"device":"d","seq":0,"uptime_ms":0,"temp_mC":-5125}')
        self.assertEqual(r.temp_mc, -5125)

    def test_not_json(self):
        with self.assertRaises(MessageError):
            parse_message(b"hello")

    def test_not_utf8(self):
        with self.assertRaises(MessageError):
            parse_message(b"\xff\xfe")

    def test_not_an_object(self):
        with self.assertRaises(MessageError):
            parse_message(b"[1,2,3]")

    def test_missing_field(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"d","seq":1,"uptime_ms":10}')

    def test_float_is_rejected(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"d","seq":1,"uptime_ms":10,"temp_mC":24.5}')

    def test_bool_is_rejected(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"d","seq":true,"uptime_ms":10,"temp_mC":1}')

    def test_empty_device_is_rejected(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"","seq":1,"uptime_ms":10,"temp_mC":1}')

    def test_implausible_temperature_is_rejected(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"d","seq":1,"uptime_ms":10,"temp_mC":900000}')

    def test_negative_seq_is_rejected(self):
        with self.assertRaises(MessageError):
            parse_message(b'{"device":"d","seq":-1,"uptime_ms":10,"temp_mC":1}')


if __name__ == "__main__":
    unittest.main()
