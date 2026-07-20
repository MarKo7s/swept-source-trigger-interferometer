# Changelog

All notable changes to pySSTri are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0] - 2026-07-17

### Added

- Pip-installable package layout (`pyproject.toml`, git-tag release via `scripts/release.py`).
- Package `pySSTri` exporting `SSTriggerInterferometer` / `feedbackmode` from `serialInterface.py`.
- Optional `notebooks` extra (matplotlib, jupyter, ipykernel) for `trigger_example.ipynb`.

### Changed

- Repo layout: firmware under `psoc5/firmware/` (formerly `Trigger/`); Eagle assets under `pcb/`.
- Class renamed from `PSOC5_trigger` to `SSTriggerInterferometer`.
