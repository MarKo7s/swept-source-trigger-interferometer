# Changelog

All notable changes to pySSTri are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Planned

- Firmware deglitching / debounce on the BPD trigger path to reject false edges from interferogram noise.

## [1.1.1] - 2026-07-28

### Added

- Zenodo archival deposit for a citable DOI (GitHub–Zenodo integration).

## [1.1.0] - 2026-07-23

### Added

- `SYS:TIMESTAMP:DELTAENC OFF|UINT8|UINT16` (aliases `0|1|2|8|16` in FW) — UART timestamp delta encoding; set may reply `OK - WARNING: …`; query returns limit warning. Python `SetTimestampsEncoding` / `GetTimestampsEncoding`.
- `LASER:SWE:STATUS?` — `1` while laser sweeping (`sw` low), `0` idle; allowed during sweep. Python `GetSweepStatus`.
- `LASER:SWE:COUNT?` / `LASER:SWE:COUNT 0|RESET` — cumulative laser sweep counter; Python `GetSweepCount` / `ResetSweepCount`.
- Optional `COM=None` with `discover()` / `connect()` — autodetect PSoC trigger via `*IDN?`.
- `pySSTri.commands` command table and `pySSTri.utilities` helpers; arg-aware auto-methods (e.g. `SetFreqDivision(n)`).
- `SIG:TRIG:TIMESTAMP?` — pull last sweep timestamps (`TSU n ov fc t0 enc` + payload); Python `GetTimestamps()`.
- `SYS:TRIG:NOT TIMESTAMP` / `5` — unsolicited timestamp frame each sweep; Python `SetModeTimestamp()` / `waitTimestamps()`.
- Firmware Polarity Low **1.3.1**: ISR Timer capture timestamps (µs), up to 4000 samples/sweep, first→last frequency, active-low **LEVEL** sweep gate.

### Changed

- Default git branch is **`main`** (was `release`); `scripts/release.py` defaults to `main`.
- TSU wire format: always `TSU <n> <ov> <fc> <t0> <enc>` (`enc` last); host decodes from the header only.
- End-of-sweep notify restored (`OFF|TIME|COUNT|FREQ|ALL|TIMESTAMP`); frequency from first→last timestamp span.
- Docs: false-trigger / interferogram-noise note; deglitching called out as future firmware work.

### Fixed

- Timestamp path uses Timer capture ISR (DMA capture path abandoned).

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
