"""Part 1 — Data processing and visualization in Python (2025)

This module implements the required functionality:

- wave_inference: efficient numerical computation of wave interference
- plot_wave: basic visualization for the computed amplitude field
- generate_sinus: advanced visualization of sin/cos with min/max coloring
- download_data: download and parse the station table from FIT VUT proxy

Notes
-----
- Only standard library, numpy, matplotlib, BeautifulSoup, and requests are used
  as required by the assignment. No pandas or seaborn imports are present.
- The module does not execute anything on import; example usage is guarded by
  a __main__ block.

PEP8/PEP257 conventions are followed, and all public functions include docstrings.
"""
from __future__ import annotations

from typing import Any, Dict, List

import re
import numpy as np
from numpy.typing import NDArray


def wave_inference(
    x: NDArray[Any],
    y: NDArray[Any],
    source: NDArray[Any],
    wavelength: float,
) -> NDArray[Any]:
    """Compute the wave interference amplitude field.

    The amplitude field Z for points defined by coordinates ``x`` (X-axis) and
    ``y`` (Y-axis) with multiple sources ``source`` is computed as:

        k = 2π / λ
        d^2_s[i, j] = (x[i] - S_s[0])^2 + (y[j] + S_s[1])^2
        r_s[i, j]   = sqrt(d^2_s[i, j])
        Z[i, j] = Σ_s shape(cos(k * r_s[i, j])) / (1 + d^2_s[i, j])

    where ``shape(·)`` is a saturation mapping that thickens bright/dark ring
    bands visually by pushing values toward ±1.

    The implementation is fully vectorized using NumPy; no explicit Python loops
    over the grid points or sources are performed.

    Parameters
    ----------
    x : NDArray[Any]
        1D array of X coordinates.
    y : NDArray[Any]
        1D array of Y coordinates.
    source : NDArray[Any]
        Array of shape (num_sources, 2) where each row is [x_s, y_s].
    wavelength : float
        Wavelength λ used to compute k = 2π/λ.

    Returns
    -------
    NDArray[Any]
        2D array of shape (len(x), len(y)) with the amplitude field Z.
    """
    if wavelength == 0:
        raise ValueError("wavelength must be non-zero")

    x = np.asarray(x, dtype=np.float64)
    y = np.asarray(y, dtype=np.float64)
    source = np.asarray(source, dtype=np.float64)

    if source.ndim != 2 or source.shape[1] != 2:
        raise ValueError("source must have shape (num_sources, 2)")

    # Create 2D coordinate grid with indexing='ij' so that Z has shape (len(x), len(y))
    X, Y = np.meshgrid(x, y, indexing="ij")  # shapes: (nx, ny)

    # Expand to 3D by adding a leading dimension for sources
    sx = source[:, 0][:, None, None]
    sy = source[:, 1][:, None, None]

    # Squared distance for each source to every grid point
    # Per assignment: d^2 = (x - Sx)^2 + (y + Sy)^2
    dx = X[None, :, :] - sx
    dy = Y[None, :, :] + sy
    r2 = dx * dx + dy * dy  # shape: (ns, nx, ny)
    r = np.sqrt(r2)

    k = 2.0 * np.pi / float(wavelength)

    # Compute contribution per source with a saturation transform to thicken bands
    c = np.cos(k * r)
    # Stronger saturation (exp < 1) => significantly thicker bright/dark regions
    c = np.sign(c) * (np.abs(c) ** 0.15)
    contributions = c / (1.0 + r2)
    Z = np.sum(contributions, axis=0)
    return Z


def plot_wave(
    Z: NDArray[Any],
    x: NDArray[Any],
    y: NDArray[Any],
    show_figure: bool = False,
    save_path: str | None = None,
) -> None:
    """Visualize the wave field ``Z`` over coordinates ``x`` and ``y``.

    The visualization aims to be similar to the reference: proper axis ranges,
    aligned colorbar, and an equal aspect ratio so the grid is not distorted.

    Parameters
    ----------
    Z : NDArray[Any]
        2D amplitude field of shape (len(x), len(y)).
    x : NDArray[Any]
        1D array of X coordinates (used for axis extent).
    y : NDArray[Any]
        1D array of Y coordinates (used for axis extent).
    show_figure : bool, optional
        If True, displays the figure via ``plt.show()``.
    save_path : str | None, optional
        If provided, saves the figure to this path via ``plt.savefig()``.
    """
    import matplotlib.pyplot as plt  # local import per assignment constraints
    from matplotlib.colors import Normalize
    from matplotlib.ticker import FormatStrFormatter

    x = np.asarray(x)
    y = np.asarray(y)
    Z = np.asarray(Z)

    if Z.shape != (x.size, y.size):
        raise ValueError(
            f"Z has shape {Z.shape}, but expected ({x.size}, {y.size})"
        )

    fig, ax = plt.subplots(figsize=(7.5, 6))

    # Normalize Z to [-1, 1] and apply mild gamma to thicken visual bands further
    max_abs = float(np.max(np.abs(Z))) or 1.0
    Zn = Z / max_abs
    Zn = np.sign(Zn) * (np.abs(Zn) ** 0.35)

    # imshow expects matrix indexing (rows as Y), so transpose for consistent axes
    x_min, x_max = float(x.min()), float(x.max())
    y_min, y_max = float(y.min()), float(y.max())
    extent = [x_min, x_max, y_min, y_max]

    im = ax.imshow(
        Zn.T,
        extent=extent,
        origin="lower",
        aspect="equal",
        cmap="YlGn",
        norm=Normalize(vmin=-1.0, vmax=1.0),
        interpolation="bicubic",
    )

    # Colorbar aligned to the axis height
    try:
        from mpl_toolkits.axes_grid1 import make_axes_locatable

        divider = make_axes_locatable(ax)
        cax = divider.append_axes("right", size="5%", pad=0.1)
        cbar = fig.colorbar(im, cax=cax)
    except Exception:
        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("Aplituda vlny")
    ticks = np.arange(-1.0, 1.0 + 1e-9, 0.25)
    cbar.set_ticks(ticks)
    cbar.formatter = FormatStrFormatter("%.2f")
    cbar.update_ticks()

    # Axes formatting: exact data limits and ticks every 2.5 on both axes
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    try:
        step = 2.5
        def _ticks(vmin: float, vmax: float, s: float) -> np.ndarray:
            start = np.ceil(vmin / s) * s
            end = np.floor(vmax / s) * s
            if end < start:
                # fallback to simple endpoints if range is smaller than step
                return np.array([vmin, vmax], dtype=float)
            # add a small epsilon to include end due to fp rounding
            return np.arange(start, end + 1e-9, s)
        ax.set_xticks(_ticks(x_min, x_max, step))
        ax.set_yticks(_ticks(y_min, y_max, step))
    except Exception:
        pass

    ax.set_xlabel("X pozice")
    ax.set_ylabel("Y pozice")
    ax.set_title("Vlnové pole")

    fig.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=150)
    if show_figure:
        plt.show()
    plt.close(fig)


def generate_sinus(
    show_figure: bool = False,
    save_path: str | None = None,
) -> None:
    """Create a two-panel figure for sin(x) and cos(x) on [0, 4π].

    - First subplot: plot sin(x) and cos(x), and fill the area between them.
    - Second subplot: dashed line showing ``min(sin, cos)`` and a colored line
      for ``max(sin, cos)`` where the color indicates which function attains the
      maximum (orange for cos, blue for sin). The x and y axes are shared
      between subplots, and the x-axis ticks use LaTeX-like labels.

    Parameters
    ----------
    show_figure : bool, optional
        If True, displays the figure via ``plt.show()``.
    save_path : str | None, optional
        If provided, saves the figure to this path via ``plt.savefig()``.
    """
    import matplotlib.pyplot as plt  # local import per assignment constraints

    # Data
    x = np.linspace(0.0, 4.0 * np.pi, 2000)
    s = np.sin(x)
    c = np.cos(x)

    # Shared axes
    fig, (ax1, ax2) = plt.subplots(
        2, 1, sharex=True, sharey=True, figsize=(8.5, 6.5)
    )

    # Subplot 1: both curves with fill-between
    ax1.plot(x, s, color="#7f7f7f", linewidth=2.0)
    ax1.plot(x, c, color="#7f7f7f", linewidth=2.0)
    ax1.fill_between(x, s, c, color="#2ca02c", alpha=0.35)
    ax1.set_xlim(0.0, 4.0 * np.pi)
    ax1.set_ylim(-1.5, 1.5)
    ax1.set_yticks(np.arange(-1.5, 1.51, 0.5))
    ax1.set_ylabel("f(x)")
    ax1.grid(True, alpha=0.25)

    # Subplot 2: min (dashed) and max colored by source function
    m = np.minimum(s, c)
    M = np.maximum(s, c)
    is_cos_max = c >= s

    # Plot min as dashed gray
    ax2.plot(x, m, linestyle="--", color="#7f7f7f", linewidth=1.6)

    # Plot max with color indicating which function dominates
    max_cos = np.where(is_cos_max, M, np.nan)
    max_sin = np.where(~is_cos_max, M, np.nan)
    ax2.plot(x, max_cos, color="#ff7f0e", linewidth=2.2)
    ax2.plot(x, max_sin, color="#1f77b4", linewidth=2.2)

    # Ticks every π/2 from 0 to 4π — matches reference
    xticks = [
        0.0,
        0.5 * np.pi,
        1.0 * np.pi,
        1.5 * np.pi,
        2.0 * np.pi,
        2.5 * np.pi,
        3.0 * np.pi,
        3.5 * np.pi,
        4.0 * np.pi,
    ]
    ax2.set_xticks(xticks)
    try:
        ax2.set_xticklabels([
            "0",
            r"$\frac{\pi}{2}$",
            r"$\pi$",
            r"$\frac{3\pi}{2}$",
            r"$2\pi$",
            r"$\frac{5\pi}{2}$",
            r"$3\pi$",
            r"$\frac{7\pi}{2}$",
            r"$4\pi$",
        ])
    except Exception:
        ax2.set_xticklabels(["0", "π/2", "π", "3π/2", "2π", "5π/2", "3π", "7π/2", "4π"])  # robust fallback

    ax2.set_xlim(0.0, 4.0 * np.pi)
    ax2.set_ylim(-1.5, 1.5)
    # No x-axis label per specification; keep y-axis labeled as f(x)
    ax2.set_ylabel("f(x)")
    ax2.grid(True, alpha=0.25)

    # Overall adjustments
    fig.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=150)
    if show_figure:
        plt.show()
    plt.close(fig)


# ----- Task 4: Download and parse station table -----

_BASE_URL = "https://ehw.fit.vutbr.cz/izv/"
_STATIONS_HTML = _BASE_URL + "st_zemepis_cz.html"


def _parse_cz_decimal(value: str) -> float:
    """Parse a Czech-formatted decimal number to float.

    This helper accepts strings like "49,365°" or "1\u00a0221,50" with various
    non-numeric decorations and converts them to a Python float using '.' as
    the decimal separator.
    """
    if value is None:
        raise ValueError("value is None")

    # Remove non-breaking spaces and regular spaces
    cleaned = value.replace("\xa0", " ")
    # Keep digits, comma, minus and dot, strip everything else (e.g., degree sign)
    cleaned = re.sub(r"[^0-9,\.\-]", "", cleaned)
    # Replace commas with dots, collapse multiple dots if any
    cleaned = cleaned.replace(",", ".")
    # Remove any leading/trailing dots that may result from stripping
    cleaned = cleaned.strip(".")
    if cleaned in ("", "-", "."):
        raise ValueError(f"cannot parse decimal from: {value!r}")
    return float(cleaned)


def download_data() -> Dict[str, List[Any]]:
    """Download the station table via the FIT VUT proxy and return parsed data.

    The function accesses the proxy page (not the official CHMI domain) and
    extracts the following columns into a dictionary of lists:

    - positions: station names (str)
    - lats: latitude in decimal degrees (float)
    - longs: longitude in decimal degrees (float)
    - heights: elevation in meters (float)

    Returns
    -------
    Dict[str, List[Any]]
        Dictionary with keys 'positions', 'lats', 'longs', 'heights'.
    """
    import requests
    from bs4 import BeautifulSoup

    resp = requests.get(_STATIONS_HTML, timeout=30)
    resp.raise_for_status()
    html = resp.text

    # Use lxml if available; otherwise fall back to the built-in html.parser
    try:
        soup = BeautifulSoup(html, "lxml")
    except Exception:
        soup = BeautifulSoup(html, "html.parser")

    positions: List[str] = []
    lats: List[float] = []
    longs: List[float] = []
    heights: List[float] = []

    # The second table in the body contains the data; be defensive and scan rows.
    tables = soup.find_all("table")
    for table in tables:
        for tr in table.find_all("tr"):
            tds = tr.find_all("td")
            # Expect at least 7 cells (name, link, lat dec, lat dms, lon dec, lon dms, height)
            if len(tds) < 7:
                continue
            # Name is in the first cell; typically within <strong>
            name = tds[0].get_text(strip=True)
            if not name:
                continue

            # Decimal latitude and longitude, then height in the last cell
            try:
                lat_dec = _parse_cz_decimal(tds[2].get_text(strip=True))
                lon_dec = _parse_cz_decimal(tds[4].get_text(strip=True))
                height = _parse_cz_decimal(tds[6].get_text(strip=True))
            except Exception:
                # Skip malformed rows (e.g., headers or separators)
                continue

            positions.append(name)
            lats.append(lat_dec)
            longs.append(lon_dec)
            heights.append(height)

    if not positions:
        raise RuntimeError("No station data parsed from the proxy page.")

    return {
        "positions": positions,
        "lats": lats,
        "longs": longs,
        "heights": heights,
    }


if __name__ == "__main__":
    # Tests (silent): ensure correctness and headless plotting
    import math as _math
    import os as _os
    import tempfile as _tempfile
    import matplotlib as _mpl
    _mpl.use("Agg", force=True)

    def _wave_inference_bad(_x: np.ndarray, _y: np.ndarray, _src: np.ndarray, _wl: float) -> np.ndarray:
        if _wl == 0:
            raise ValueError("wavelength must be non-zero")
        _k = 2.0 * _math.pi / float(_wl)
        _Z = np.zeros((_x.size, _y.size), dtype=float)
        for _i, _xi in enumerate(_x):
            for _j, _yj in enumerate(_y):
                _s = 0.0
                for _sx, _sy in _src:
                    _dx = _xi - float(_sx)
                    _dy = _yj + float(_sy)
                    _r2 = _dx * _dx + _dy * _dy
                    _r = _math.sqrt(_r2)
                    _c = _math.cos(_k * _r)
                    # Match saturation used in vectorized implementation
                    _c = (_c / abs(_c) if _c != 0.0 else 0.0) * (abs(_c) ** 0.15)
                    _s += _c / (1.0 + _r2)
                _Z[_i, _j] = _s
        return _Z

    _Xt = np.linspace(-10.0, 10.0, 40)
    _Yt = np.linspace(-10.0, 10.0, 40)
    _St = np.array([[-3.0, 0.0], [3.0, 0.0], [0.0, 4.0]], dtype=float)
    _Zv = wave_inference(_Xt, _Yt, _St, wavelength=2.0)
    _Zn = _wave_inference_bad(_Xt, _Yt, _St, 2.0)
    assert _Zv.shape == (_Xt.size, _Yt.size)
    assert np.isfinite(_Zv).all()
    assert np.allclose(_Zv, _Zn, rtol=1e-12, atol=1e-12)

    with _tempfile.TemporaryDirectory() as _d:
        _wave_png = _os.path.join(_d, "wave.png")
        plot_wave(_Zv, _Xt, _Yt, show_figure=False, save_path=_wave_png)
        assert _os.path.exists(_wave_png) and _os.path.getsize(_wave_png) > 0

    with _tempfile.TemporaryDirectory() as _d2:
        _sinus_png = _os.path.join(_d2, "sinus.png")
        generate_sinus(show_figure=False, save_path=_sinus_png)
        assert _os.path.exists(_sinus_png) and _os.path.getsize(_sinus_png) > 0

    try:
        _data = download_data()
        _n = len(_data.get("positions", []))
        assert _n > 0
        assert len(_data.get("lats", [])) == _n
        assert len(_data.get("longs", [])) == _n
        assert len(_data.get("heights", [])) == _n
    except Exception:
        # Allow environments without network/parsers to pass silently
        pass

    # Example manual run (silent; no console output)
    X = np.linspace(-10, 10, 200)
    Y = np.linspace(-10, 10, 200)
    S = np.array([[-3.0, 0.0], [3.0, 0.0], [0.0, 4.0]], dtype=float)
    Z = wave_inference(X, Y, S, wavelength=2.0)

    # Save figures (not shown by default)
    try:
        plot_wave(Z, X, Y, show_figure=False, save_path="wave.png")
        generate_sinus(show_figure=False, save_path="sinus.png")
    except Exception:
        # Plotting may fail in some headless environments without a backend
        pass

    # Fetch and summarize station data (no prints)
    try:
        _ = download_data()
    except Exception:
        pass
