"""Serial interface for the ModeLab swept-source interferometer PSOC5 camera trigger."""

from importlib.metadata import PackageNotFoundError, version

try:
    __version__ = version("pySSTri")
except PackageNotFoundError:
    __version__ = "1.0.0"

from .commands import feedbackmode, notifymode, timestamp_deltaenc
from .serialInterface import SSTriggerInterferometer

__all__ = [
    "SSTriggerInterferometer",
    "notifymode",
    "feedbackmode",
    "timestamp_deltaenc",
    "__version__",
]
