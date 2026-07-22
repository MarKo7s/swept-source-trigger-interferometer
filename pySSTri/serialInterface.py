import serial

# SYS:TRIG:NOT payload / enable (0=OFF means no UART notify at end of sweep)
notifymode = {"OFF": 0, "TIME": 1, "COUNT": 2, "FREQ": 3, "ALL": 4}

# Backwards-friendly alias for older imports
feedbackmode = notifymode


class SSTriggerInterferometer:
    def __init__(self, COM, BR=115200, timeout=1):
        self.COM = COM
        self.BR = BR
        try:
            self.ser = serial.Serial(self.COM, self.BR, timeout=timeout)
        except serial.serialutil.SerialException as exc:
            raise ConnectionError(f"Connection to {COM} port failed") from exc

        # Keep old attribute name used by some notebooks
        self.PSOC5_serial_COM = self.ser

        # method_name -> (wire_command, note)
        self.commands = {
            "ID": ("*IDN?", ""),
            "GetFrequency": ("SIG:TRIG:EVENTS:FREQ?", "Hz"),
            "GetTriggeredFrames": ("SIG:TRIG:EVENTS:COUNT?", ""),
            "GetSweepTime": ("LASER:SWE:TIME?", "us"),
            "GetMode": ("SYS:TRIG:NOT?", notifymode),
            "SetModeNormal": ("SYS:TRIG:NOT OFF", "notify off"),
            "SetModeTime": ("SYS:TRIG:NOT TIME", ""),
            "SetModeCount": ("SYS:TRIG:NOT COUNT", ""),
            "SetModeFrequency": ("SYS:TRIG:NOT FREQ", ""),
            "SetModeAll": ("SYS:TRIG:NOT ALL", ""),
        }
        self._generate_methods()

    @staticmethod
    def encode(cmd: str) -> bytes:
        return f"{cmd}\r\n".encode("utf-8")

    @staticmethod
    def decode(msg: bytes) -> str:
        return msg.decode("utf-8").strip()

    def ask(self, cmd: str) -> str:
        """Send a command and wait for one reply line (OK / ERROR: n / value)."""
        self.ser.write(self.encode(cmd))
        return self.decode(self.ser.readline())

    def scpi(self, cmd: str) -> str:
        return self.ask(cmd)

    @staticmethod
    def is_error(reply: str) -> bool:
        return reply.startswith("ERROR:")

    def SetFreqDivision(self, n: int) -> str:
        return self.ask(f"SIG:TRIG:DIV {n}")

    def GetFreqDivision(self) -> str:
        return self.ask("SIG:TRIG:DIV?")

    def waitForSignal(self) -> str:
        """Block until next unsolicited sweep notify line (SYS:TRIG:NOT != OFF)."""
        return self.decode(self.ser.readline())

    def pullbuffer(self):
        return self.ser.readline()

    def SetSerialTimeOut(self, t):
        self.ser.timeout = t

    def GetSerialTimeOut(self):
        return self.ser.timeout

    def flushSerialBuffer(self):
        self.ser.reset_input_buffer()

    def CloseSerialConnection(self):
        self.ser.close()
        print("Serial communication with PSOC closed")

    def discoverMethods(self):
        for idx, (name, (cmd, note)) in enumerate(self.commands.items()):
            print(f"{idx} - {name} - {cmd} - {note}")
        print(f"{len(self.commands)} - SetFreqDivision - SIG:TRIG:DIV <n>")
        print(f"{len(self.commands) + 1} - GetFreqDivision - SIG:TRIG:DIV?")

    def _generate_methods(self):
        for name, (cmd, _note) in self.commands.items():

            def make(cmd=cmd):
                def method(self):
                    return self.ask(cmd)

                return method

            setattr(self, name, make().__get__(self, self.__class__))
