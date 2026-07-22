# Changelog

All notable changes to pySSTri are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0] - 2026-07-22

### Added

- Pip-installable package layout (`pyproject.toml`, git-tag release via `scripts/release.py`).
- Package `pySSTri` exporting `SSTriggerInterferometer` / `notifymode` from `serialInterface.py`.
- Optional `notebooks` extra (matplotlib, jupyter, ipykernel) for `trigger_example.ipynb`.
- SCPI-style UART protocol v1.0 (CRLF): `*IDN?`, `SIG:TRIG:DIV`, `SIG:TRIG:EVENTS:COUNT?`, `SIG:TRIG:EVENTS:FREQ?`, `LASER:SWE:TIME?`, `SYS:TRIG:NOT`.
- Firmware identity string with `FW_VERSION` and `FW_DATE` via `*IDN?`.
- Error replies `ERROR: 0` (unsupported/invalid) and `ERROR: 1` (laser sweeping).
- Sets acknowledge with `OK`; host uses write+read for all commands.

### Changed

- Repo layout: firmware under `psoc5/firmware/` (formerly `Trigger/`); Eagle assets under `pcb/`.
- Class renamed from `PSOC5_trigger` to `SSTriggerInterferometer`.
- Clean break from legacy commands (`freq_int`, `SetMode_*` wire strings, bare-number divider, leading `\r`).
- Default frequency division on reset is `2`.
- End-of-sweep host sync uses single `SYS:TRIG:NOT OFF|TIME|COUNT|FREQ|ALL` (replaces separate wait/mode).
