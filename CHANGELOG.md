# Changelog

All notable changes to pySSTri are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `LASER:SWE:COUNT?` / `LASER:SWE:COUNT 0|RESET` — cumulative laser sweep counter; Python `GetSweepCount` / `ResetSweepCount`.
- Optional `COM=None` with `discover()` / `connect()` — autodetect PSoC trigger via `*IDN?`.
- `pySSTri.commands` command table and `pySSTri.utilities` helpers; arg-aware auto-methods (e.g. `SetFreqDivision(n)`).

### Changed

- Command dictionary and `encode` / `decode` / `is_error` / method attachment moved out of `serialInterface.py`.

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
