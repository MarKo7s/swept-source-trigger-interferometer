import serial
from serial.tools import list_ports

from .commands import COMMANDS
from .utilities import _unpack, attach_methods, decode, encode

import re
import numpy as np

_IDN_MARK = "PSOC"


class SSTriggerInterferometer:
    def __init__(self, COM=None, BR=115200, timeout=1):
        self.COM = COM
        self.BR = BR
        self.timeout = timeout
        self.ser = None
        self.PSOC5_serial_COM = None

        self.commands = COMMANDS
        attach_methods(self, self.commands)

        if COM is not None:
            self.connect()

    def _probe(self, port: str) -> str | None:
        """Return *IDN? reply if this port looks like the PSoC trigger."""
        try:
            with serial.Serial(port, self.BR, timeout=self.timeout) as ser:
                ser.reset_input_buffer()
                ser.write(encode("*IDN?"))
                idn = decode(ser.readline())
        except (serial.SerialException, OSError):
            return None
        if _IDN_MARK in idn.upper():
            return idn
        return None

    def discover(self) -> str:
        """Scan serial ports for a PSoC trigger; set and return self.COM."""
        for info in list_ports.comports():
            idn = self._probe(info.device)
            if idn is None:
                continue
            self.COM = info.device
            print(f"PSOC trigger with ID '{idn}' found in port {self.COM}")
            return self.COM
        raise ConnectionError("No PSOC trigger found on any serial port")

    def connect(self):
        """Open serial. If COM is None, autodiscover first."""
        if self.ser is not None and self.ser.is_open:
            return self

        if self.COM is None:
            self.discover()

        try:
            self.ser = serial.Serial(self.COM, self.BR, timeout=self.timeout)
        except serial.SerialException as exc:
            raise ConnectionError(f"Connection to {self.COM} failed") from exc

        self.PSOC5_serial_COM = self.ser
        return self

    def ask(self, cmd: str) -> str:
        """Send a command and wait for one reply line (OK / ERROR: n / value)."""
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected — call connect() first")
        self.ser.write(encode(cmd))
        return decode(self.ser.readline())

    def scpi(self, cmd: str) -> str:
        return self.ask(cmd)

    def waitForSignal(self) -> str:
        """Block until next unsolicited sweep notify line (SYS:TRIG:NOT != OFF)."""
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected — call connect() first")
        return decode(self.ser.readline())

    def waitTimestamps(self, timeout_s=5.0):
        """
        Wait for one sweep dump:
        header: TS <n> <overflow>\\r\\n
        payload: n * uint32 little-endian (raw Timer captures)
        Returns (ts: np.ndarray dtype uint32, overflow: int)
        """
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected")
        old = self.ser.timeout
        self.ser.timeout = timeout_s
        try:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            m = re.match(r"TS\s+(\d+)\s+(\d+)", line)
            if not m:
                raise RuntimeError(f"Bad TS header: {line!r}")
            n = int(m.group(1))
            overflow = int(m.group(2))
            raw = self.ser.read(n * 4)
            if len(raw) != n * 4:
                raise RuntimeError(f"Short read: got {len(raw)}, expected {n * 4}")
            ts = np.frombuffer(raw, dtype="<u4").copy()
            return ts, overflow
        finally:
            self.ser.timeout = old

    def pullbuffer(self):
        return self.ser.readline()

    def SetSerialTimeOut(self, t):
        self.timeout = t
        if self.ser is not None:
            self.ser.timeout = t

    def GetSerialTimeOut(self):
        return self.ser.timeout if self.ser is not None else self.timeout

    def flushSerialBuffer(self):
        self.ser.reset_input_buffer()

    def CloseSerialConnection(self):
        if self.ser is not None:
            self.ser.close()
            self.ser = None
            self.PSOC5_serial_COM = None
        print("Serial communication with PSOC closed")

    def discoverMethods(self):
        for idx, (name, spec) in enumerate(self.commands.items()):
            scpi, note, takes_arg = _unpack(spec)
            wire = f"{scpi} <value>" if takes_arg else scpi
            print(f"{idx} - {name} - {wire} - {note}")
