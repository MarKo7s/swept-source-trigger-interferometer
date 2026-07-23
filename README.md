# SWEPT SOURCE INTERFEROMETER CAMERA TRIGGER

![alt text](images/swept_source_trigger_white.png)

## Package layout

```
.
├── pySSTri/                Python serial package
│   ├── __init__.py         exports SSTriggerInterferometer
│   └── serialInterface.py  SSTriggerInterferometer serial API
├── psoc5/
│   └── firmware/           PSOC Creator / firmware project (was Trigger/)
├── pcb/                    Eagle schematics / board
├── trigger_example.ipynb   usage example
├── images/
├── scripts/release.py      git-tag release helper
├── pyproject.toml          package version + deps
└── CHANGELOG.md
```

## PCB schematics

Schematics available in Autodesk Eagle: `pcb/Eagle_project/Trigger_PSOC5LP`

## Firmware / tools

1. [Install PSOC programmer](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.psocprogrammer) (update firmware), then close it.
2. [Install PSOC creator](https://www.infineon.com/cms/en/design-support/tools/sdk/psoc-software/psoc-creator/)
3. Open the workspace under `psoc5/firmware/PSOC_trigger_firmware_Trigger_Polarity_Low` with PSOC Creator. Program the board via **Debug → Program**.

### Serial protocol (firmware v1.0)

Lines use CRLF (`\r\n`). Sets reply `OK` or `ERROR: <n>`; queries reply a value.

| Command | Meaning |
|---------|---------|
| `*IDN?` | Identify (`FW` + `DATE`) |
| `SIG:TRIG:DIV <n>` / `SIG:TRIG:DIV?` | Set/get camera trigger divider (default `2`) |
| `SIG:TRIG:EVENTS:COUNT?` | Last sweep trigger count |
| `SIG:TRIG:EVENTS:FREQ?` | Last trigger frequency [Hz] |
| `LASER:SWE:TIME?` | Last sweep duration [us] |
| `LASER:SWE:COUNT?` | Cumulative laser sweep count since reset |
| `LASER:SWE:COUNT 0` / `RESET` | Clear sweep count (`OK`; other args → `ERROR: 0`) |
| `SYS:TRIG:NOT OFF\|TIME\|COUNT\|FREQ\|ALL` | End-of-sweep UART notify (or `0..4`) |
| `SYS:TRIG:NOT?` | Current notify mode |

Errors: `ERROR: 0` unsupported/invalid; `ERROR: 1` laser sweeping.

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
board.SetModeCount()              # SYS:TRIG:NOT COUNT
board.flushSerialBuffer()
print(board.waitForSignal())      # unsolicited line each sweep
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
4. Run the release script from the repo root (this repo’s default branch is `release`):

```bash
python scripts/release.py --from-changelog
```

The script reads the version from `pyproject.toml`, pushes `release`, creates annotated git tag `vX.Y.Z`, and pushes the tag. With `--from-changelog`, the tag message is taken from the matching `CHANGELOG.md` section. You can also pass a custom message with `--message "..."` (overrides `--from-changelog`).

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
