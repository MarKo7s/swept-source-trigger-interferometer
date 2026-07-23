# SWEPT SOURCE INTERFEROMETER CAMERA TRIGGER

![alt text](images/swept_source_trigger_white.png)

## Package layout

```
.
├── pySSTri/                Python serial package
│   ├── __init__.py         exports SSTriggerInterferometer
│   └── serialInterface.py  SSTriggerInterferometer serial API
├── psoc5/
│   └── firmware/           PSoC Creator firmware projects (polarity / laser variants)
├── pcb/                    Eagle schematics / board
├── trigger_example.ipynb   usage example
├── images/
├── scripts/release.py      git-tag release helper
├── pyproject.toml          package version + deps
└── CHANGELOG.md
```

## PCB schematics

Schematics available in Autodesk Eagle: `pcb/Eagle_project/Trigger_PSOC5LP`

## Firmware

Firmware is **variant-specific**: trigger polarity, laser wiring, and feature set differ per PSoC Creator project under `psoc5/firmware/`. More variants will be added over time (other polarities, laser trigger styles, etc.). Pick the project that matches your hardware.

| Project folder | Notes |
|----------------|--------|
| `PSOC_trigger_firmware_Trigger_Polarity_Low` | **Current production (FW 1.3.1)** — **active-low LEVEL** sweep gate, timestamps |
| `PSOC_trigger_firmware_DEBUG` | Debug / experimental |

### Sweep gate (Polarity Low) — LEVEL, not edge

This production image treats the laser sweep input (`sw`) as **active-low LEVEL**:

- **Sweeping** while `sw` is held **low** (`sw_Read() == 0`) for the whole sweep window.
- **Idle** when `sw` is high.
- It is **not** a falling-edge one-shot; the entire low interval is the acquisition window.

`LASER:SWE:STATUS?` returns `1` while low / `0` while idle. Other host commands during a sweep reply `ERROR: 1` (except `STATUS?`).

Program with [PSoC Programmer](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.psocprogrammer) / [PSoC Creator](https://www.infineon.com/cms/en/design-support/tools/sdk/psoc-software/psoc-creator/): open the matching workspace, then **Debug → Program**.

Identity string (`*IDN?`) reports `FW` version and `DATE` for the programmed image.

### Serial protocol

Lines use CRLF (`\r\n`). Sets reply `OK` or `OK - WARNING: …` or `ERROR: <n>`; queries reply a value.

| Command | Meaning |
|---------|---------|
| `*IDN?` | Identify (`FW` + `DATE`) |
| `SIG:TRIG:DIV <n>` / `SIG:TRIG:DIV?` | Set/get camera trigger divider (default `2`) |
| `SIG:TRIG:EVENTS:COUNT?` | Last sweep trigger count |
| `SIG:TRIG:EVENTS:FREQ?` | Last trigger frequency [Hz] (from first→last timestamp span) |
| `SIG:TRIG:TIMESTAMP?` | Last sweep timestamps (see below); not allowed while sweeping |
| `LASER:SWE:TIME?` | Last sweep duration [µs] |
| `LASER:SWE:COUNT?` | Cumulative laser sweep count since reset |
| `LASER:SWE:COUNT 0` / `RESET` | Clear sweep count (`OK`; other args → `ERROR: 0`) |
| `LASER:SWE:STATUS?` | `1` = sweeping, `0` = idle (allowed during sweep) |
| `SYS:TIMESTAMP:DELTAENC OFF\|UINT8\|UINT16` | Timestamp UART encoding (`0`/`1`/`2` or `8`/`16` also accepted) |
| `SYS:TIMESTAMP:DELTAENC?` | Current encoding + limit warning (see below) |
| `SYS:TRIG:NOT OFF\|TIME\|COUNT\|FREQ\|ALL\|TIMESTAMP` | End-of-sweep UART notify (or `0..5`) |
| `SYS:TRIG:NOT?` | Current notify mode |

Errors: `ERROR: 0` unsupported/invalid; `ERROR: 1` laser sweeping.

### Timestamps (Polarity Low / timestamp-capable FW)

**Capture path:** each camera trigger edge is captured into a free-running 32-bit Timer (24 MHz). ISR `isr_timeStamp` drains the Timer capture FIFO into RAM during the sweep. At sweep end (`INT_SW`):

1. Finalize remaining FIFO samples.
2. Convert each raw capture to **µs from sweep start**: `(period − capture) / 24`.
3. Compute trigger frequency from the **first and last** stamps: `(n − 1) / (t_last − t_first)` [Hz].
4. Send via UART if queried or if `SYS:TRIG:NOT TIMESTAMP` (encoding from `SYS:TIMESTAMP:DELTAENC`).

**Capacity:** up to **4000** timestamps per sweep (`TS_MAX`). Extra edges set `ov` and are dropped.

**False triggers / interferogram noise:** comparator edges on a noisy BPD / interferogram can produce **spurious camera triggers** (extra timestamps, inflated count/frequency, irregular Δt). A hardware **deglitching** (debounce) filter on the trigger path is the intended fix; it is **not in the current Polarity Low image** and needs to be added in a future firmware/schematic revision. Until then, raise comparator threshold / improve front-end SNR, or post-filter pathological gaps in software.

#### Wire frame (raw serial — no Python required)

ASCII header line, then a binary payload. Field order is fixed:

```
TSU <n> <ov> <fc> <t0> <enc>\r\n
<payload bytes>
```

| Field | Meaning |
|-------|---------|
| `n` | Number of absolute timestamps represented |
| `ov` | `1` if buffer overflowed and/or a delta was clamped |
| `fc` | Hardware frequency-counter event count for the sweep |
| `t0` | First sample time [µs] (always present; `0` if `n==0`) |
| `enc` | **Last field:** `0` = absolute uint32; `1` = uint8 deltas; `2` = uint16 LE deltas |

| `enc` | Payload |
|-------|---------|
| `0` | `n × uint32` little-endian absolute µs |
| `1` | `(n − 1) × uint8` deltas; reconstruct `t[0]=t0`, `t[i]=t[i−1]+d[i−1]` |
| `2` | `(n − 1) × uint16` little-endian deltas; same reconstruction |

If `n ≤ 1` and `enc ≠ 0`, payload is empty (use header `t0` when `n == 1`).

**Oversized deltas:** gaps larger than 255 µs (UINT8) or 65535 µs (UINT16) are **clamped** and `ov` is set to `1`. Prefer a wider encoding or `OFF`.

#### `SYS:TIMESTAMP:DELTAENC`

Aliases are handled **in firmware** (`OFF|0`, `UINT8|1|8`, `UINT16|2|16`).

| Set arg | Reply |
|---------|--------|
| `OFF` / `0` | `OK` |
| `UINT8` / `1` / `8` | `OK - WARNING: max_dt_us=255 min_freq_hz=3922` |
| `UINT16` / `2` / `16` | `OK - WARNING: max_dt_us=65535 min_freq_hz=15` |

| Query | Reply |
|-------|--------|
| OFF | `OFF` |
| UINT8 | `UINT8 WARNING max_dt_us=255 min_freq_hz=3922` |
| UINT16 | `UINT16 WARNING max_dt_us=65535 min_freq_hz=15` |

Limits are **maximum gap / minimum rate** for lossless deltas: UINT8 needs consecutive gaps ≤ 255 µs (trigger **≥ ~3.9 kHz**); UINT16 ≤ 65535 µs (**≥ ~15 Hz**). Faster trains always fit; slower ones clamp.

#### Raw-serial pull example

```
SYS:TIMESTAMP:DELTAENC 1\r\n
→ OK - WARNING: max_dt_us=255 min_freq_hz=3922

(wait until LASER:SWE:STATUS? → 0)

SIG:TRIG:TIMESTAMP?\r\n
→ TSU 512 0 512 1234 1\r\n
→ then 511 bytes of uint8 deltas
```

Decode from the header alone (`enc` last): `t[0]=t0`; for each delta `d`, `t.append(t[-1]+d)`.

Python (`GetTimestamps` / `waitTimestamps`) does the same; `SetTimestampsEncoding` only configures the device (no local encode state).

## Python interface

Use `SSTriggerInterferometer` to talk to the board over serial and change trigger settings. Example notebook: `trigger_example.ipynb`.

Baudrate defaults to 115200. Pass a COM port to connect immediately, or omit it and call `connect()` (autodiscovers via `*IDN?`):

```python
from pySSTri import SSTriggerInterferometer

board = SSTriggerInterferometer()   # or COM="COM6" to skip scan
board.connect()                     # finds PSOC, stores board.COM, opens port
board.discoverMethods()
board.ID()
board.SetFreqDivision(4)          # -> OK
print(board.SetTimestampsEncoding(1))  # -> OK - WARNING: … (aliases in FW)
board.SetModeCount()              # SYS:TRIG:NOT COUNT
board.flushSerialBuffer()
print(board.waitForSignal())      # unsolicited line each sweep
print(board.GetSweepStatus())     # "0" idle / "1" sweeping

# Absolute times in us (decoded from TSU header enc; max 4000 / sweep):
# board.SetModeTimestamp()
# ts_us, ov, fc = board.waitTimestamps()
ts_us, ov, fc = board.GetTimestamps()
```

## Installation

### From GitHub (tagged release)

```bash
pip install "pySSTri @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.0.0"
```

With the example notebook extras:

```bash
pip install "pySSTri[notebooks] @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.0.0"
```

### Local development (editable install)

```bash
git clone git@github.com:MarKo7s/swept-source-trigger-interferometer.git
cd swept-source-trigger-interferometer
pip install -e ".[notebooks]"
```

### Conda environment

```bash
conda create -n pySSTri_env python=3.11 -y
conda activate pySSTri_env
pip install -e ".[notebooks]"
```

Register the Jupyter kernel (once):

```bash
python -m ipykernel install --user --name pySSTri_env --display-name "Python (pySSTri_env)"
```

`requirements.txt` remains available for legacy workflows; prefer `pip install -e ".[notebooks]"`.

## Developer notes

### Versioning

The package version is defined in **one place only**: `pyproject.toml` → `[project].version`.

Do **not** edit `pySSTri/__init__.py` on each release. `pySSTri.__version__` is read from pip metadata after install (`importlib.metadata`).

Check the installed version:

```bash
pip show pySSTri
python -c "import pySSTri; print(pySSTri.__version__)"
```

Use [semantic versioning](https://semver.org/): `MAJOR.MINOR.PATCH`.

### Releasing a new version

1. Add an entry for the new version at the top of `CHANGELOG.md`.
2. Bump `version` in `pyproject.toml`.
3. Commit all changes (including the changelog).
4. Run the release script from the repo root (default branch is `main`):

```bash
python scripts/release.py --from-changelog
```

The script reads the version from `pyproject.toml`, pushes `main`, creates annotated git tag `vX.Y.Z`, and pushes the tag. With `--from-changelog`, the tag message is taken from the matching `CHANGELOG.md` section. You can also pass a custom message with `--message "..."` (overrides `--from-changelog`).

After that, others can install with:

```bash
pip install "pySSTri @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@vX.Y.Z"
```

Dry run (no git changes):

```bash
python scripts/release.py --from-changelog --dry-run
```

Optional: create a GitHub Release page with the same notes (`gh` CLI required):

```bash
gh release create vX.Y.Z --title "pySSTri X.Y.Z" --notes-file CHANGELOG.md
```

**Requirements before release:** clean working tree (all changes committed); tag `vX.Y.Z` must not already exist on GitHub.
