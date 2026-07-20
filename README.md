# SWEPT SOURCE INTERFEROMETER CAMERA TRIGGER

![alt text](images/swept_source_trigger_white.png)

## Package layout

```
.
├── psoc5.py                PSOC5_trigger serial API
├── trigger_example.ipynb   usage example
├── Trigger/                PCB / PSOC Creator / Eagle project
├── images/
├── scripts/release.py      git-tag release helper
├── pyproject.toml          package version + deps
└── CHANGELOG.md
```

## PCB schematics

Schematics available in Autodesk Eagle: `Trigger/Eagle_project/Trigger_PSCOS5LP`

## Firmware / tools

1. [Install PSOC programmer](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.psocprogrammer) (update firmware), then close it.
2. [Install PSOC creator](https://www.infineon.com/cms/en/design-support/tools/sdk/psoc-software/psoc-creator/)
3. Open `Trigger_PSOC5.cywrk` inside the Psoc_creator_project folder with PSOC Creator. Program the board via **Debug → Program**.

## Python interface

Use `trigger.PSOC5_trigger` to talk to the board over serial and change trigger settings. Example notebook: `trigger_example.ipynb`.

Baudrate defaults to 115200; pass the COM port when creating the object:

```python
import trigger

board = trigger.PSOC5_trigger(COM="COM6")
board.discoverMethods()
board.ID()
```

## Installation

### From GitHub (tagged release)

```bash
pip install "trigger @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.0.0"
```

With the example notebook extras:

```bash
pip install "trigger[notebooks] @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@v1.0.0"
```

### Local development (editable install)

```bash
git clone git@github.com:MarKo7s/swept-source-trigger-interferometer.git
cd swept-source-trigger-interferometer
pip install -e ".[notebooks]"
```

### Conda environment

```bash
conda create -n trigger_env python=3.11 -y
conda activate trigger_env
pip install -e ".[notebooks]"
```

Register the Jupyter kernel (once):

```bash
python -m ipykernel install --user --name trigger_env --display-name "Python (trigger_env)"
```

`requirements.txt` remains available for legacy workflows; prefer `pip install -e ".[notebooks]"`.

## Developer notes

### Versioning

The package version is defined in **one place only**: `pyproject.toml` → `[project].version`.

Do **not** edit `__init__.py` on each release. `trigger.__version__` is read from pip metadata after install (`importlib.metadata`).

Check the installed version:

```bash
pip show trigger
python -c "import trigger; print(trigger.__version__)"
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
pip install "trigger @ git+https://github.com/MarKo7s/swept-source-trigger-interferometer.git@vX.Y.Z"
```

Dry run (no git changes):

```bash
python scripts/release.py --from-changelog --dry-run
```

Optional: create a GitHub Release page with the same notes (`gh` CLI required):

```bash
gh release create vX.Y.Z --title "trigger X.Y.Z" --notes-file CHANGELOG.md
```

**Requirements before release:** clean working tree (all changes committed); tag `vX.Y.Z` must not already exist on GitHub.
