"""Serial interface for the ModeLab swept-source interferometer PSOC5 camera trigger."""

from importlib.metadata import PackageNotFoundError, version

try:
    __version__ = version("pySSTri")
except PackageNotFoundError:
    __version__ = "1.0.0"

from .serialInterface import SSTriggerInterferometer, feedbackmode

__all__ = [
    "SSTriggerInterferometer",
    "feedbackmode",
    "__version__",
]
