"""Serial interface for the ModeLab swept-source interferometer PSOC5 camera trigger."""

from importlib.metadata import PackageNotFoundError, version

try:
    __version__ = version("trigger")
except PackageNotFoundError:
    __version__ = "1.0.0"

from .psoc5 import PSOC5_trigger, feedbackmode

__all__ = [
    "PSOC5_trigger",
    "feedbackmode",
    "__version__",
]
