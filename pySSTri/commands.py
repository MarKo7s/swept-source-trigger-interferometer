# SYS:TRIG:NOT payload / enable (0=OFF means no UART notify at end of sweep)
notifymode = {"OFF": 0, "TIME": 1, "COUNT": 2, "FREQ": 3, "ALL": 4}

# Backwards-friendly alias for older imports
feedbackmode = notifymode

# method_name -> (scpi, note) or (scpi, note, True) when the method takes one value
COMMANDS = {
    "ID": ("*IDN?", ""),
    "GetFrequency": ("SIG:TRIG:EVENTS:FREQ?", "Hz"),
    "GetTriggeredFrames": ("SIG:TRIG:EVENTS:COUNT?", ""),
    "GetSweepTime": ("LASER:SWE:TIME?", "us"),
    "GetSweepCount": ("LASER:SWE:COUNT?", "total sweeps since reset"),
    "GetMode": ("SYS:TRIG:NOT?", notifymode),
    "SetModeNormal": ("SYS:TRIG:NOT OFF", "notify off"),
    "SetModeTime": ("SYS:TRIG:NOT TIME", ""),
    "SetModeCount": ("SYS:TRIG:NOT COUNT", ""),
    "SetModeFrequency": ("SYS:TRIG:NOT FREQ", ""),
    "SetModeAll": ("SYS:TRIG:NOT ALL", ""),
    "GetFreqDivision": ("SIG:TRIG:DIV?", ""),
    "SetFreqDivision": ("SIG:TRIG:DIV", "<n>", True),
    "ResetSweepCount": ("LASER:SWE:COUNT 0", "reset"),
}
