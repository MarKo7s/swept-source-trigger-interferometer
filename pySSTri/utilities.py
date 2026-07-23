def encode(cmd: str) -> bytes:
    return f"{cmd}\r\n".encode("utf-8")


def decode(msg: bytes) -> str:
    return msg.decode("utf-8").strip()


def is_error(reply: str) -> bool:
    return reply.startswith("ERROR:")


def _unpack(spec):
    """Return (scpi, note, takes_arg) from a COMMANDS entry."""
    if len(spec) == 2:
        scpi, note = spec
        return scpi, note, False
    scpi, note, takes_arg = spec
    return scpi, note, bool(takes_arg)


def attach_methods(obj, commands: dict) -> None:
    """Bind ask()-backed methods from a COMMANDS dict onto obj."""
    for name, spec in commands.items():
        scpi, _note, takes_arg = _unpack(spec)

        if takes_arg:

            def make(scpi=scpi):
                def method(self, value):
                    return self.ask(f"{scpi} {value}")

                return method

        else:

            def make(scpi=scpi):
                def method(self):
                    return self.ask(scpi)

                return method

        setattr(obj, name, make().__get__(obj, type(obj)))
