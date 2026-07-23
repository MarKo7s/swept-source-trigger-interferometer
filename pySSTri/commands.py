# SYS:TRIG:NOT payload / enable (0=OFF means no UART notify at end of sweep)
notifymode = {
    "OFF": 0,
    "TIME": 1,
    "COUNT": 2,
    "FREQ": 3,
    "ALL": 4,
    "TIMESTAMP": 5,
}

# SYS:TIMESTAMP:DELTAENC
timestamp_deltaenc = {
    "OFF": 0,
    "UINT8": 1,
    "UINT16": 2,
}

# Backwards-friendly alias for older imports
feedbackmode = notifymode

# method_name -> (scpi, note) or (scpi, note, True) when the method takes one value
# Binary timestamp pull / SetTimestampsEncoding live on SSTriggerInterferometer.
COMMANDS = {
    "ID": ("*IDN?", ""),
    "GetFrequency": ("SIG:TRIG:EVENTS:FREQ?", "Hz"),
    "GetTriggeredFrames": ("SIG:TRIG:EVENTS:COUNT?", ""),
    "GetSweepTime": ("LASER:SWE:TIME?", "us"),
    "GetSweepCount": ("LASER:SWE:COUNT?", "total sweeps since reset"),
    "GetSweepStatus": ("LASER:SWE:STATUS?", "1=sweeping, 0=idle"),
    "GetMode": ("SYS:TRIG:NOT?", notifymode),
    "SetModeNormal": ("SYS:TRIG:NOT OFF", "notify off"),
    "SetModeTime": ("SYS:TRIG:NOT TIME", ""),
    "SetModeCount": ("SYS:TRIG:NOT COUNT", ""),
    "SetModeFrequency": ("SYS:TRIG:NOT FREQ", ""),
    "SetModeAll": ("SYS:TRIG:NOT ALL", ""),
    "SetModeTimestamp": ("SYS:TRIG:NOT TIMESTAMP", "unsolicited TSU dump each sweep (us)"),
    "GetTimestampsEncoding": ("SYS:TIMESTAMP:DELTAENC?", "OFF or UINT8/16 WARNING…"),
    "SetTimestampsEncoding": (
        "SYS:TIMESTAMP:DELTAENC",
        "OFF|UINT8|UINT16 or 0|1|2 (aliases in FW)",
        True,
    ),
    "GetFreqDivision": ("SIG:TRIG:DIV?", ""),
    "SetFreqDivision": ("SIG:TRIG:DIV", "<n>", True),
    "ResetSweepCount": ("LASER:SWE:COUNT 0", "reset"),
}
