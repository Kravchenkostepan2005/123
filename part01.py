"""Part 1 – Data processing and visualization in Python (2025).

This module implements the required functions:

- wave_inference: Efficient, vectorized computation of wave interference
  over a 2D grid for multiple point sources.
- plot_wave: Visualization helper that renders the interference field.
- generate_sinus: Advanced visualization of sin and cos with shaded areas
  and styled axes, over the domain [0, 4π].
- download_data: Downloads and parses the meteorological station listing
  from the FIT VUT IŽV mirror pages under https://ehw.fit.vutbr.cz/izv.

Notes:
- Do not import this file for side-effects; the module performs no actions
  on import. Example usage is available under the main guard.
- Only standard libraries and the explicitly allowed third-party packages
  (numpy, matplotlib, requests, beautifulsoup4) are used.
- The implementation strives to be PEP 8 and PEP 257 compliant.
"""
from __future__ import annotations

from typing import Any, Dict, List, Optional

import math
import re
from urllib.parse import urljoin

import numpy as np
from numpy.typing import NDArray

# Third-party imports for plotting and data download are optional until used
# to avoid import-time failures if a user only needs numerical parts.
try:  # pragma: no cover - optional at import time
    import matplotlib.pyplot as plt  # type: ignore
except Exception:  # pragma: no cover - defer import errors until use
    plt = None  # type: ignore

try:  # pragma: no cover - optional at import time
    import requests  # type: ignore
    from bs4 import BeautifulSoup  # type: ignore
except Exception:  # pragma: no cover - defer import errors until use
    requests = None  # type: ignore
    BeautifulSoup = None  # type: ignore


__all__ = [
    "wave_inference",
    "plot_wave",
    "generate_sinus",
    "download_data",
]


def wave_inference(
    x: NDArray[Any],
    y: NDArray[Any],
    source: NDArray[Any],
    wavelength: float,
) -> NDArray[Any]:
    """Compute wave interference amplitude field for multiple point sources.

    For each grid point (i, j) defined by coordinates x[i], y[j] and every source
    s in ``source`` (with coordinates [sx, sy]), the amplitude contribution is

        cos(k * d) / (1 + d)

    where d is the Euclidean distance from (x[i], y[j]) to (sx, sy) and
    k = 2π / wavelength.

    The result is the sum of contributions across all sources for each grid point.

    The implementation is fully vectorized with NumPy (no Python loops over grid
    elements or sources), for efficiency.

    Parameters
    ----------
    x
        1D array of x-coordinates (shape: [nx]).
    y
        1D array of y-coordinates (shape: [ny]).
    source
        2D array of shape [n_sources, 2], each row is [sx, sy].
    wavelength
        Wavelength λ. Must be a positive, non-zero float.

    Returns
    -------
    NDArray[Any]
        2D array (shape [nx, ny]) of resulting amplitudes Z.

    Raises
    ------
    ValueError
        If wavelength is not positive or source has invalid shape.
    """
    if wavelength <= 0:
        raise ValueError("wavelength must be a positive, non-zero float")

    sources = np.asarray(source, dtype=float)
    if sources.ndim != 2 or sources.shape[1] != 2:
        raise ValueError("source must be of shape [n_sources, 2]")

    x = np.asarray(x, dtype=float)
    y = np.asarray(y, dtype=float)

    # Create 2D grid with shape [nx, ny]
    X, Y = np.meshgrid(x, y, indexing="ij")

    # Broadcast to 3D tensors: [nx, ny, n_sources]
    sx = sources[:, 0][None, None, :]  # [1, 1, n_sources]
    sy = sources[:, 1][None, None, :]

    dx = X[..., None] - sx
    dy = Y[..., None] - sy

    # Euclidean distance for each source
    d = np.sqrt(dx * dx + dy * dy)

    k = 2.0 * math.pi / float(wavelength)

    # Contribution per source and reduction across sources
    contrib = np.cos(k * d) / (1.0 + d)
    Z = np.sum(contrib, axis=2)
    return Z


def plot_wave(
    Z: NDArray[Any],
    x: NDArray[Any],
    y: NDArray[Any],
    show_figure: bool = False,
    save_path: Optional[str] = None,
) -> None:
    """Plot a 2D wave interference field with a colorbar.

    The image is rendered with axes extents matching the coordinate arrays.

    Parameters
    ----------
    Z
        2D array (shape [len(x), len(y)]) with amplitudes to visualize.
    x
        1D array of x-coordinates used to compute Z.
    y
        1D array of y-coordinates used to compute Z.
    show_figure
        If True, display the figure via ``plt.show()``.
    save_path
        If provided, path to save the figure via ``plt.savefig()``.
    """
    if plt is None:
        raise RuntimeError(
            "matplotlib is required for plotting. Please install matplotlib."
        )

    Z = np.asarray(Z)
    x = np.asarray(x)
    y = np.asarray(y)

    if Z.ndim != 2:
        raise ValueError("Z must be a 2D array")

    # We used indexing='ij', so Z[i, j] corresponds to (x[i], y[j]).
    extent = [float(x.min()), float(x.max()), float(y.min()), float(y.max())]

    fig, ax = plt.subplots(figsize=(7, 6))
    im = ax.imshow(
        Z.T,  # transpose to keep axes in the intuitive x/y order in imshow
        extent=extent,
        origin="lower",
        cmap="viridis",
        aspect="equal",
        interpolation="nearest",
    )
    ax.set_xlabel("x")
    ax.set_ylabel("y")

    # Colorbar aligned to the main axis
    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label("amplitude")

    ax.set_xlim(extent[0], extent[1])
    ax.set_ylim(extent[2], extent[3])
    fig.tight_layout()

    if save_path:
        fig.savefig(save_path, dpi=150)
    if show_figure:
        plt.show()
    else:
        plt.close(fig)


def generate_sinus(
    show_figure: bool = False,
    save_path: Optional[str] = None,
) -> None:
    """Generate a figure with two subplots of sin(x) and cos(x) over [0, 4π].

    First subplot: both sin and cos curves, with the area between them shaded.

    Second subplot: dashed line of the element-wise minimum and a colored
    element-wise maximum; color the maximum line based on the dominating
    function (orange where max is cos, blue where max is sin). The two
    subplots share both x and y axes. The x-axis uses LaTeX labels with
    π markings from 0 to 4π.

    Parameters
    ----------
    show_figure
        If True, display the figure via ``plt.show()``.
    save_path
        If provided, path to save the figure via ``plt.savefig()``.
    """
    if plt is None:
        raise RuntimeError(
            "matplotlib is required for plotting. Please install matplotlib."
        )

    x = np.linspace(0.0, 4.0 * math.pi, 2000)
    sinx = np.sin(x)
    cosx = np.cos(x)

    fig, (ax1, ax2) = plt.subplots(
        2, 1, sharex=True, sharey=True, figsize=(8, 6)
    )

    # Subplot 1: sin and cos with filled area between
    ax1.plot(x, sinx, label="sin(x)", color="#1f77b4")
    ax1.plot(x, cosx, label="cos(x)", color="#ff7f0e")
    ax1.fill_between(
        x,
        sinx,
        cosx,
        color="#9ecae1",
        alpha=0.35,
        label="area between",
    )
    ax1.legend(loc="upper right", frameon=False)
    ax1.grid(True, alpha=0.3)

    # Subplot 2: dashed minimum and colored maximum
    min_xy = np.minimum(sinx, cosx)
    max_xy = np.maximum(sinx, cosx)

    ax2.plot(x, min_xy, linestyle="--", color="#4d4d4d", label="min(sin, cos)")

    mask_cos = cosx >= sinx
    # Plot maximum in two colored segments depending on which function dominates
    ax2.plot(x[mask_cos], max_xy[mask_cos], color="#ff7f0e", label="max (cos)")
    ax2.plot(x[~mask_cos], max_xy[~mask_cos], color="#1f77b4", label="max (sin)")

    ax2.legend(loc="upper right", frameon=False)
    ax2.grid(True, alpha=0.3)

    # Styling: ticks and LaTeX labels on x-axis at multiples of π/2
    ticks = np.arange(0.0, 4.0 * math.pi + 1e-9, math.pi / 2.0)
    tick_labels = []
    for t in ticks:
        k = round(t / math.pi * 2)  # number of half-pi units
        if k == 0:
            tick_labels.append(r"$0$")
        elif k == 8:
            tick_labels.append(r"$4\pi$")
        elif k % 2 == 0:
            # integer multiples of π
            m = k // 2
            tick_labels.append(rf"${m}\pi$")
        else:
            # odd -> π/2 + nπ
            m = k // 2
            if m == 0:
                tick_labels.append(r"$\frac{\pi}{2}$")
            else:
                tick_labels.append(rf"${m}\frac{{\pi}}{{2}}$")

    ax2.set_xticks(ticks)
    ax2.set_xticklabels(tick_labels)

    for ax in (ax1, ax2):
        ax.set_xlim(0.0, 4.0 * math.pi)
        ax.set_ylim(-1.1, 1.1)
        ax.set_xlabel(r"$x$")
        ax.set_ylabel("value")

    fig.tight_layout()
    if save_path:
        fig.savefig(save_path, dpi=150)
    if show_figure:
        plt.show()
    else:
        plt.close(fig)


def download_data() -> Dict[str, List[Any]]:
    """Download and parse the meteorological station table from the IŽV site.

    The data must be retrieved from the IŽV mirror at ``https://ehw.fit.vutbr.cz/izv``.
    Accessing CHMI directly is prohibited by the assignment. This function tries
    to be resilient to the page's "original" structure:

    1. Fetch the station page ``/izv/stanice.html``.
    2. Search the page and loaded scripts for JSON endpoints.
    3. Attempt to fetch and parse any discovered JSON.
    4. If JSON is not discovered, attempt to parse an HTML table if present.

    The output schema is a dictionary with the following keys, each mapping to a
    list of values covering all stations in consistent order:

    - "positions": list[str] – station names
    - "lats": list[float] – latitudes
    - "longs": list[float] – longitudes
    - "heights": list[float] – elevations in meters

    Returns
    -------
    Dict[str, List[Any]]
        Parsed table in column-oriented format.

    Raises
    ------
    RuntimeError
        If the data cannot be retrieved or parsed.
    """
    if requests is None or BeautifulSoup is None:
        raise RuntimeError(
            "requests and beautifulsoup4 are required. Please install them."
        )

    base = "https://ehw.fit.vutbr.cz"
    page_url = f"{base}/izv/stanice.html"

    try:
        resp = requests.get(page_url, timeout=30)
        resp.raise_for_status()
    except Exception as exc:
        # If page fetch fails (e.g., network is blocked), still try a small set
        # of common fallback JSON endpoints used by the site.
        candidates = [
            f"{base}/izv/stanice.json",
            f"{base}/izv/data/stanice.json",
            f"{base}/izv/data/stations.json",
            f"{base}/izv/assets/stanice.json",
        ]
        data = _try_fetch_json_candidates(candidates)
        if data is None:
            raise RuntimeError(
                f"Failed to fetch station page and tried JSON fallbacks: {exc}"
            ) from exc
        return data

    html = resp.text
    soup = BeautifulSoup(html, "lxml")

    # Collect candidate URLs from inline text and external scripts
    candidates: List[str] = []

    # 1) URLs in page HTML
    for m in re.finditer(r"https?://[^\"'<>\s]+", html, re.IGNORECASE):
        url = m.group(0)
        if any(key in url.lower() for key in ("stan", "stati", "json")):
            candidates.append(url)

    # 2) External scripts' sources
    script_srcs = [
        urljoin(page_url, s.get("src"))
        for s in soup.find_all("script")
        if s.get("src")
    ]

    # 3) Inline scripts text
    inline_scripts = [s.string or "" for s in soup.find_all("script") if not s.get("src")]

    # Search inline scripts for candidate endpoints
    for script_text in inline_scripts:
        for m in re.finditer(r"https?://[^\"'<>\s]+", script_text, re.IGNORECASE):
            url = m.group(0)
            if any(key in url.lower() for key in ("stan", "stati", "json")):
                candidates.append(url)
        # Look for fetch('...') patterns
        for m in re.finditer(r"fetch\(([^)]+)\)", script_text, re.IGNORECASE):
            raw = m.group(1).strip().strip("\"' ")
            if raw:
                candidates.append(urljoin(page_url, raw))

    # Search external scripts
    for src in script_srcs:
        try:
            r = requests.get(src, timeout=20)
            r.raise_for_status()
        except Exception:
            continue
        txt = r.text
        for m in re.finditer(r"https?://[^\"'<>\s]+", txt, re.IGNORECASE):
            url = m.group(0)
            if any(key in url.lower() for key in ("stan", "stati", "json")):
                candidates.append(url)
        for m in re.finditer(r"fetch\(([^)]+)\)", txt, re.IGNORECASE):
            raw = m.group(1).strip().strip("\"' ")
            if raw:
                candidates.append(urljoin(page_url, raw))

    # Add a few known fallbacks
    candidates.extend(
        [
            f"{base}/izv/stanice.json",
            f"{base}/izv/data/stanice.json",
            f"{base}/izv/data/stations.json",
            f"{base}/izv/assets/stanice.json",
        ]
    )

    # Deduplicate while preserving order
    seen = set()
    unique_candidates = []
    for c in candidates:
        if c not in seen:
            seen.add(c)
            unique_candidates.append(c)

    data = _try_fetch_json_candidates(unique_candidates)
    if data is not None:
        return data

    # Try to parse HTML table as a last resort
    table_data = _try_parse_html_table(soup)
    if table_data is not None:
        return table_data

    raise RuntimeError("Failed to locate station data in page or scripts.")


# ------------------------------
# Helpers for download_data
# ------------------------------

def _try_fetch_json_candidates(candidates: List[str]) -> Optional[Dict[str, List[Any]]]:
    """Attempt to fetch and parse any JSON endpoint in the list.

    Returns the parsed dictionary on success, otherwise None.
    """
    if requests is None:
        return None

    for url in candidates:
        try:
            r = requests.get(url, timeout=20)
            r.raise_for_status()
            payload = r.json()
            parsed = _parse_station_json(payload)
            if parsed is not None:
                return parsed
        except Exception:
            continue
    return None


def _parse_station_json(payload: Any) -> Optional[Dict[str, List[Any]]]:
    """Parse a station JSON payload into the expected dict format.

    Attempts to handle a few possible schema variants. Returns None if the
    payload cannot be understood as a station listing.
    """
    rows: List[dict] = []

    if isinstance(payload, dict):
        # Common cases: { "stations": [...] } or a flat mapping of id -> {...}
        if "stations" in payload and isinstance(payload["stations"], list):
            rows = [r for r in payload["stations"] if isinstance(r, dict)]
        else:
            # Try mapping of arbitrary keys to station dicts
            cand = [v for v in payload.values() if isinstance(v, (list, dict))]
            if cand and isinstance(cand[0], list):
                rows = [r for r in cand[0] if isinstance(r, dict)]
            elif cand and isinstance(cand[0], dict):
                rows = [v for v in payload.values() if isinstance(v, dict)]
    elif isinstance(payload, list):
        rows = [r for r in payload if isinstance(r, dict)]

    if not rows:
        return None

    positions: List[str] = []
    lats: List[float] = []
    longs: List[float] = []
    heights: List[float] = []

    def _get_first_key(d: dict, keys: List[str]) -> Any:
        for k in keys:
            if k in d:
                return d[k]
        # Try case-insensitive lookup
        lower = {str(k).lower(): v for k, v in d.items()}
        for k in keys:
            lk = k.lower()
            if lk in lower:
                return lower[lk]
        return None

    for r in rows:
        name = _get_first_key(
            r,
            ["name", "nazev", "position", "stanice", "title", "place"],
        )
        lat = _get_first_key(r, ["lat", "latitude", "y", "lat_deg"])  # type: ignore[arg-type]
        lon = _get_first_key(r, ["lon", "long", "lng", "longitude", "x", "long_deg"])  # type: ignore[arg-type]
        h = _get_first_key(r, ["height", "elevation", "alt", "altitude", "z"])  # type: ignore[arg-type]

        if name is None or lat is None or lon is None:
            # Skip rows that do not resemble a station entry
            continue

        try:
            positions.append(str(name).strip())
            lats.append(float(lat))
            longs.append(float(lon))
            if h is None:
                heights.append(float("nan"))
            else:
                heights.append(float(h))
        except Exception:
            # Skip malformed entries
            continue

    if not positions:
        return None

    return {
        "positions": positions,
        "lats": lats,
        "longs": longs,
        "heights": heights,
    }


def _try_parse_html_table(soup: Any) -> Optional[Dict[str, List[Any]]]:
    """Attempt to parse a station HTML table if JSON endpoints are unavailable."""
    if BeautifulSoup is None:
        return None

    table = soup.find("table")
    if table is None:
        return None

    # Extract headers to map columns
    headers = []
    thead = table.find("thead")
    if thead:
        for th in thead.find_all("th"):
            headers.append(th.get_text(strip=True))
    if not headers:
        first_tr = table.find("tr")
        if first_tr:
            headers = [th.get_text(strip=True) for th in first_tr.find_all("th")]

    rows = []
    tbody = table.find("tbody") or table
    for tr in tbody.find_all("tr"):
        cols = [td.get_text(strip=True) for td in tr.find_all(["td", "th"])]
        if cols:
            rows.append(cols)

    # Heuristic mapping by header keywords (Czech/English variants)
    idx_name = _find_header_index(headers, ["stanice", "pozice", "název", "name", "position"])  # type: ignore[arg-type]
    idx_lat = _find_header_index(headers, ["šířka", "lat", "latitude"])  # type: ignore[arg-type]
    idx_lon = _find_header_index(headers, ["délka", "lon", "long", "longitude"])  # type: ignore[arg-type]
    idx_h = _find_header_index(headers, ["nadmoř", "výška", "height", "elevation", "alt"])  # type: ignore[arg-type]

    positions: List[str] = []
    lats: List[float] = []
    longs: List[float] = []
    heights: List[float] = []

    for cols in rows:
        try:
            if idx_name is None or idx_lat is None or idx_lon is None:
                continue
            name = cols[idx_name]
            lat_s = cols[idx_lat].replace(",", ".")
            lon_s = cols[idx_lon].replace(",", ".")
            h_s = cols[idx_h].replace(",", ".") if idx_h is not None and idx_h < len(cols) else "nan"

            positions.append(str(name).strip())
            lats.append(float(lat_s))
            longs.append(float(lon_s))
            heights.append(float(h_s))
        except Exception:
            continue

    if not positions:
        return None

    return {
        "positions": positions,
        "lats": lats,
        "longs": longs,
        "heights": heights,
    }


def _find_header_index(headers: List[str], keywords: List[str]) -> Optional[int]:
    """Find the index of the first header containing any of the keywords."""
    if not headers:
        return None
    low = [h.lower() for h in headers]
    for i, h in enumerate(low):
        for k in keywords:
            if k.lower() in h:
                return i
    return None


if __name__ == "__main__":  # pragma: no cover
    # Minimal smoke tests/demos (safe to skip if dependencies are missing).
    try:
        X = np.linspace(-10, 10, 200)
        Y = np.linspace(-10, 10, 200)
        S = np.array([[-3.0, 0.0], [3.0, 0.0], [0.0, 4.0]], dtype=float)
        Z = wave_inference(X, Y, S, wavelength=2.0)
        try:
            plot_wave(Z, X, Y, show_figure=False, save_path="wave.png")
            print("Saved wave.png")
        except Exception as e:
            print("Plotting unavailable:", e)

        try:
            generate_sinus(show_figure=False, save_path="sinus.png")
            print("Saved sinus.png")
        except Exception as e:
            print("Sinus plotting unavailable:", e)

    except Exception as exc:
        print("Demo failed:", exc)
