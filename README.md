# Swept-source interferometer camera trigger

![Board](images/swept_source_trigger_white.png)

PSoC 5LP board that gates on the laser sweep, divides BPD/camera triggers, and (on the Polarity Low firmware) timestamps each trigger edge in microseconds. Hosts talk over UART (115200 baud, CRLF). The Python package **pySSTri** wraps that protocol.

---

## Installation (Python)

**Tagged release:**

```bash
pip install "pySSTri @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.1.0"
```

With notebook extras:

```bash
pip install "pySSTri[notebooks] @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.1.0"
```

**Editable (dev):**

```bash
git clone git@github.com:MarKo7s/swept-source-trigger-interferometer.git
cd swept-source-trigger-interferometer
pip install -e ".[notebooks]"
```

**Conda:**

```bash
conda create -n pySSTri_env python=3.11 -y
conda activate pySSTri_env
pip install -e ".[notebooks]"
python -m ipykernel install --user --name pySSTri_env --display-name "Python (pySSTri_env)"
```

---

## How to operate

There are two layers:

| Layer | Use when |
|-------|----------|
| **Firmware (UART / SCPI)** | Any host language, raw serial, or writing your own driver |
| **Python (`pySSTri`)** | Lab scripts / notebooks — preferred for day-to-day use |

Same baudrate (115200) and same commands either way. Python methods map 1:1 onto SCPI (e.g. `SetFreqDivision(4)` → `SIG:TRIG:DIV 4`).

---

## Firmware

### Which project to flash

Firmware is **variant-specific** (polarity / laser). Pick the folder under `psoc5/firmware/` that matches your hardware:

| Project | Role |
|---------|------|
| `PSOC_trigger_firmware_Trigger_Polarity_Low` | **Production** (FW 1.3.1) — active-low LEVEL sweep gate + timestamps |
| `PSOC_trigger_firmware_DEBUG` | Experimental |

Tools: [PSoC Programmer](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.psocprogrammer) / [PSoC Creator](https://www.infineon.com/cms/en/design-support/tools/sdk/psoc-software/psoc-creator/) → open project → **Debug → Program**.

`*IDN?` returns firmware version and date.

### Sweep gate (Polarity Low)

The sweep input `sw` is **active-low LEVEL**, not a falling-edge pulse:

- **Sweeping** while `sw` is held low
- **Idle** while `sw` is high

While sweeping, most commands return `ERROR: 1`. Exception: `LASER:SWE:STATUS?` (`1` = sweeping, `0` = idle).

### Command reference

Lines end with `\r\n`. Sets reply `OK` (or `OK - WARNING: …`). Queries reply a value. Errors: `ERROR: 0` (bad command), `ERROR: 1` (busy / sweeping).

| Command | Meaning |
|---------|---------|
| `*IDN?` | Identify |
| `SIG:TRIG:DIV <n>` / `?` | Camera trigger divider (default `2`) |
| `SIG:TRIG:EVENTS:COUNT?` | Trigger count last sweep |
| `SIG:TRIG:EVENTS:FREQ?` | Mean trigger rate [Hz] (first→last timestamp) |
| `SIG:TRIG:TIMESTAMP?` | Pull timestamp buffer (idle only) |
| `LASER:SWE:TIME?` | Sweep duration [µs] |
| `LASER:SWE:COUNT?` / `0` / `RESET` | Cumulative sweeps / clear |
| `LASER:SWE:STATUS?` | `1` sweeping / `0` idle |
| `SYS:TRIG:NOT OFF\|TIME\|COUNT\|FREQ\|ALL\|TIMESTAMP` | End-of-sweep notify (`0`…`5`) |
| `SYS:TIMESTAMP:DELTAENC OFF\|UINT8\|UINT16` | How timestamps are packed on the wire (`0`/`1`/`2` also OK) |
| `SYS:TIMESTAMP:DELTAENC?` | Current encoding + rate limit warning |

### Timestamps

Each camera-trigger edge is stamped by a 24 MHz Timer during the sweep (ISR fill, up to **4000** samples). Values are **µs from sweep start**.

**Typical flow**

1. Optionally set packing: `SYS:TIMESTAMP:DELTAENC UINT16` (see delta encoding below).
2. Run a sweep (or enable `SYS:TRIG:NOT TIMESTAMP` for an automatic dump each sweep).
3. When idle: `SIG:TRIG:TIMESTAMP?` → one ASCII header line + binary payload.

**Wire header** (always the same fields):

```
TSU <n> <ov> <fc> <t0> <enc>\r\n
```

| Field | Meaning |
|-------|---------|
| `n` | Number of timestamps |
| `ov` | `1` if buffer overflowed or a delta was clamped |
| `fc` | Hardware event count for the sweep |
| `t0` | First sample [µs] |
| `enc` | Packing mode (`0` / `1` / `2`) — always last |

Then the **payload** depends on `enc`:

| `enc` | Name | Payload | Host reconstruct |
|-------|------|---------|------------------|
| `0` | OFF (absolute) | `n × uint32` LE absolute µs | use as-is |
| `1` | UINT8 deltas | `(n−1) × uint8` gaps | `t[0]=t0`, then accumulate |
| `2` | UINT16 deltas | `(n−1) × uint16` LE gaps | same accumulate |

Delta encoding shrinks UART traffic when consecutive gaps are small. Choose a mode whose **max gap** fits your laser:

| Mode | Max gap | Safe if trigger rate |
|------|---------|----------------------|
| UINT8 | 255 µs | **≥ ~3.9 kHz** |
| UINT16 | 65535 µs | **≥ ~15 Hz** |
| OFF | (full uint32) | any |

If a gap is too large for the mode, firmware **clamps** it and sets `ov=1` (lossy). For ~3 kHz SS sources, prefer **UINT16** or **OFF**, not UINT8.

Setting the mode returns a warning with those limits, e.g.:

```
SYS:TIMESTAMP:DELTAENC 2
→ OK - WARNING: max_dt_us=65535 min_freq_hz=15
```

**False triggers:** noisy interferogram / BPD edges can create extra timestamps. A hardware **deglitch** filter is planned but not in this firmware yet — raise comparator threshold / SNR, or filter bad gaps in software for now.

PCB: Eagle project under `pcb/Eagle_project/Trigger_PSOC5LP`.

---

## Python interface (`pySSTri`)

Example notebook: `trigger_example.ipynb`.

### Connect

```python
from pySSTri import SSTriggerInterferometer

board = SSTriggerInterferometer()   # or COM="COM6"
board.connect()                     # autodiscovers via *IDN? if needed
board.discoverMethods()             # list wrappers
print(board.ID())
```

### Everyday control

```python
board.SetFreqDivision(2)
board.SetModeCount()                # notify: trigger count each sweep
board.flushSerialBuffer()
print(board.waitForSignal())        # blocks for next notify line
print(board.GetSweepStatus())       # "0" / "1"
print(board.GetFrequency(), board.GetSweepTime())
```

### Timestamps (decoded to absolute µs for you)

```python
board.SetTimestampsEncoding(2)      # UINT16 deltas on the wire (FW aliases OK)
# board.SetModeTimestamp()          # optional: auto-dump each sweep
# ts, ov, fc = board.waitTimestamps()

ts, ov, fc = board.GetTimestamps()  # after a sweep, while idle
# ts: uint32 array [µs], regardless of enc in the header
```

`GetTimestamps` / `waitTimestamps` read the `TSU` header’s `enc` field and unpack the payload — you always get absolute times.

---

## Repository layout

```
pySSTri/          Python package
psoc5/firmware/   PSoC Creator projects
pcb/              Eagle schematics
trigger_example.ipynb
scripts/release.py
```

---

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

Firmware has its own identity (`FW_VERSION` / `FW_DATE` in `main.c`, reported by `*IDN?`). Bump that when you change the PSoC image; it is independent of the pySSTri package version.

Default git branch is **`main`**.

### Releasing a new version

1. Add a `## [X.Y.Z] - YYYY-MM-DD` section at the top of `CHANGELOG.md` (keep `## [Unreleased]` above it for WIP notes).
2. Set `version = "X.Y.Z"` in `pyproject.toml`.
3. Commit everything (clean working tree).
4. From the repo root:

```bash
python scripts/release.py --from-changelog
```

The script reads the version from `pyproject.toml`, pushes `main`, creates annotated git tag `vX.Y.Z`, and pushes the tag. With `--from-changelog`, the tag message is taken from the matching `CHANGELOG.md` section. Override with `--message "..."` if needed.

Dry run (no git changes):

```bash
python scripts/release.py --from-changelog --dry-run
```

Optional GitHub Release page (`gh` CLI):

```bash
gh release create vX.Y.Z --title "pySSTri X.Y.Z" --notes-file CHANGELOG.md
```

After release, others install with:

```bash
pip install "pySSTri @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@vX.Y.Z"
```

**Requirements:** clean working tree; tag `vX.Y.Z` must not already exist on GitHub.
