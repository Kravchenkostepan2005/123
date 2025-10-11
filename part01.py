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
        d_s[i, j] = (x[i] - S_s[0])^2 + (y[j] - S_s[1])^2
        Z[i, j] = Σ_s cos(k * d_s[i, j]) / (1 + d_s[i, j])

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
    dx = X[None, :, :] - sx
    dy = Y[None, :, :] - sy
    d2 = dx * dx + dy * dy  # shape: (ns, nx, ny)

    k = 2.0 * np.pi / float(wavelength)

    # Compute contribution per source and reduce (sum over sources)
    contributions = np.cos(k * d2) / (1.0 + d2)
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

    x = np.asarray(x)
    y = np.asarray(y)
    Z = np.asarray(Z)

    if Z.shape != (x.size, y.size):
        raise ValueError(
            f"Z has shape {Z.shape}, but expected ({x.size}, {y.size})"
        )

    fig, ax = plt.subplots(figsize=(7.5, 6))

    # imshow expects matrix indexing (rows as Y), so transpose Z for consistent axes
    extent = [float(x.min()), float(x.max()), float(y.min()), float(y.max())]
    im = ax.imshow(
        Z.T,
        extent=extent,
        origin="lower",
        aspect="equal",
        cmap="viridis",
        interpolation="nearest",
    )

    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("Amplitude")

    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_title("Wave interference amplitude")

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
    ax1.plot(x, s, label="sin(x)", color="#1f77b4", linewidth=1.8)
    ax1.plot(x, c, label="cos(x)", color="#ff7f0e", linewidth=1.8)
    ax1.fill_between(x, s, c, color="#cccccc", alpha=0.5)
    ax1.set_ylabel("value")
    ax1.legend(loc="upper right")
    ax1.grid(True, alpha=0.2)

    # Subplot 2: min (dashed) and max colored by source function
    m = np.minimum(s, c)
    M = np.maximum(s, c)
    is_cos_max = c >= s

    # Plot min as dashed gray
    ax2.plot(x, m, linestyle="--", color="#666666", linewidth=1.6, label="min(sin, cos)")

    # Plot max with color indicating which function dominates
    max_cos = np.where(is_cos_max, M, np.nan)
    max_sin = np.where(~is_cos_max, M, np.nan)
    ax2.plot(x, max_cos, color="#ff7f0e", linewidth=2.0, label="max (cos)")
    ax2.plot(x, max_sin, color="#1f77b4", linewidth=2.0, label="max (sin)")

    # Ticks: 0, π, 2π, 3π, 4π (use Unicode labels for robustness)
    xticks = [0.0, np.pi, 2.0 * np.pi, 3.0 * np.pi, 4.0 * np.pi]
    ax2.set_xticks(xticks)
    ax2.set_xticklabels(["0", "π", "2π", "3π", "4π"])  # avoid mathtext dependency

    ax2.set_xlabel("x")
    ax2.set_ylabel("value")
    ax2.grid(True, alpha=0.2)

    # Overall adjustments
    fig.suptitle("Sinusoidal functions: sin(x) and cos(x)")
    fig.tight_layout(rect=[0, 0, 1, 0.97])

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
    # Simple CLI so the script is useful when executed directly.
    import argparse
    from pathlib import Path

    parser = argparse.ArgumentParser(description="Part01 utilities: plotting and data download")
    parser.add_argument("--wave", action="store_true", help="Generate wave interference plot")
    parser.add_argument("--sinus", action="store_true", help="Generate sinus/cos figure")
    parser.add_argument("--download", action="store_true", help="Download station data and print a summary")
    parser.add_argument("--show", action="store_true", help="Show figure windows (if GUI available)")
    parser.add_argument("--wave-save", type=str, default=None, help="Path to save the wave plot image")
    parser.add_argument("--sinus-save", type=str, default=None, help="Path to save the sinus plot image")
    parser.add_argument("--verbose", action="store_true", help="Print info messages (paths, summaries)")
    args = parser.parse_args()

    # Default behavior: if no flags, save both plots to PNG (robust for non-GUI envs)
    run_default = not (args.wave or args.sinus or args.download)

    if args.wave or run_default:
        X = np.linspace(-10, 10, 200)
        Y = np.linspace(-10, 10, 200)
        S = np.array([[-3.0, 0.0], [3.0, 0.0], [0.0, 4.0]], dtype=float)
        Z = wave_inference(X, Y, S, wavelength=2.0)
        wave_out = args.wave_save or ("wave.png" if (run_default or not args.show) else None)
        try:
            plot_wave(Z, X, Y, show_figure=args.show, save_path=wave_out)
            if wave_out and args.verbose:
                print(f"Saved wave plot to {Path(wave_out).resolve()}")
        except Exception as e:
            if args.verbose:
                print("Wave plot generation failed:", e)

    if args.sinus or run_default:
        sinus_out = args.sinus_save or ("sinus.png" if (run_default or not args.show) else None)
        try:
            generate_sinus(show_figure=args.show, save_path=sinus_out)
            if sinus_out and args.verbose:
                print(f"Saved sinus plot to {Path(sinus_out).resolve()}")
        except Exception as e:
            if args.verbose:
                print("Sinus plot generation failed:", e)

    if args.download:
        try:
            data = download_data()
            print(f"Parsed stations: {len(data['positions'])}")
            if data["positions"]:
                print(
                    "First:",
                    data["positions"][0],
                    data["lats"][0],
                    data["longs"][0],
                    data["heights"][0],
                )
        except Exception as e:
            print("download_data() failed:", e)
