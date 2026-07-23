import serial
from serial.tools import list_ports

from .commands import COMMANDS
from .utilities import _unpack, attach_methods, decode, encode

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
        """Block until next unsolicited notify line (not for TIMESTAMP mode)."""
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected — call connect() first")
        return decode(self.ser.readline())

    def _read_ts_frame(self, timeout_s=10.0):
        """TSU <n> <ov> <fc> <t0> <enc> + payload → (ts_us, overflow, fc)."""
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected")
        old = self.ser.timeout
        self.ser.timeout = timeout_s
        try:
            line = self.ser.readline().decode("ascii", errors="replace").strip()
            if line.startswith("ERROR:"):
                raise RuntimeError(line)
            parts = line.split()
            if len(parts) < 6 or parts[0] != "TSU":
                raise RuntimeError(f"Bad TS header: {line!r}")
            n = int(parts[1])
            overflow = int(parts[2])
            fc = int(parts[3])
            t0 = int(parts[4])
            enc = int(parts[5])

            if enc == 0:
                nbytes = n * 4
                raw = self.ser.read(nbytes) if n else b""
                if len(raw) != nbytes:
                    raise RuntimeError(f"Short read: {len(raw)}/{nbytes}")
                return np.frombuffer(raw, dtype="<u4").copy(), overflow, fc

            if n == 0:
                return np.array([], dtype=np.uint32), overflow, fc
            if n == 1:
                return np.array([t0], dtype=np.uint32), overflow, fc

            nd = n - 1
            if enc == 1:
                nbytes = nd
                raw = self.ser.read(nbytes)
                if len(raw) != nbytes:
                    raise RuntimeError(f"Short read: {len(raw)}/{nbytes}")
                deltas = np.frombuffer(raw, dtype=np.uint8)
            elif enc == 2:
                nbytes = nd * 2
                raw = self.ser.read(nbytes)
                if len(raw) != nbytes:
                    raise RuntimeError(f"Short read: {len(raw)}/{nbytes}")
                deltas = np.frombuffer(raw, dtype="<u2")
            else:
                raise RuntimeError(f"Unknown enc={enc} in {line!r}")

            ts = np.empty(n, dtype=np.uint32)
            ts[0] = t0
            ts[1:] = t0 + np.cumsum(deltas.astype(np.uint32))
            return ts, overflow, fc
        finally:
            self.ser.timeout = old

    def waitTimestamps(self, timeout_s=10.0):
        """Next unsolicited TSU frame (SYS:TRIG:NOT TIMESTAMP), values in us."""
        return self._read_ts_frame(timeout_s)

    def GetTimestamps(self, timeout_s=10.0):
        """SIG:TRIG:TIMESTAMP? → (ts_us, overflow, fc), absolute us."""
        if self.ser is None or not self.ser.is_open:
            raise ConnectionError("Not connected")
        self.ser.write(encode("SIG:TRIG:TIMESTAMP?"))
        return self._read_ts_frame(timeout_s)

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
        print("— GetTimestamps() / waitTimestamps() — TSU binary frame")
