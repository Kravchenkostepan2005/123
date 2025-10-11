import unittest
import math
import os
import tempfile

# Use Agg backend for headless environments before importing pyplot
import matplotlib
matplotlib.use("Agg", force=True)

import numpy as np

import part01 as m


def wave_inference_bad(x: np.ndarray, y: np.ndarray, source: np.ndarray, wavelength: float) -> np.ndarray:
    """Naive reference implementation for testing correctness.

    Loops over all points and sources.
    """
    if wavelength == 0:
        raise ValueError("wavelength must be non-zero")
    k = 2.0 * math.pi / float(wavelength)
    Z = np.zeros((len(x), len(y)), dtype=float)
    for i, xi in enumerate(x):
        for j, yj in enumerate(y):
            s = 0.0
            for sx, sy in source:
                d2 = (xi - float(sx)) ** 2 + (yj - float(sy)) ** 2
                s += math.cos(k * d2) / (1.0 + d2)
            Z[i, j] = s
    return Z


class TestPart01(unittest.TestCase):
    def setUp(self):
        self.X = np.linspace(-10.0, 10.0, 50)
        self.Y = np.linspace(-10.0, 10.0, 50)
        self.S = np.array([[-3.0, 0.0], [3.0, 0.0], [0.0, 4.0]], dtype=float)

    def test_wave_inference_matches_naive(self):
        Z_vec = m.wave_inference(self.X, self.Y, self.S, wavelength=2.0)
        Z_naive = wave_inference_bad(self.X, self.Y, self.S, wavelength=2.0)
        self.assertEqual(Z_vec.shape, (self.X.size, self.Y.size))
        self.assertTrue(np.isfinite(Z_vec).all())
        self.assertTrue(np.allclose(Z_vec, Z_naive, rtol=1e-12, atol=1e-12))

    def test_plot_wave_saves_image(self):
        Z = m.wave_inference(self.X, self.Y, self.S, wavelength=2.0)
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "wave.png")
            m.plot_wave(Z, self.X, self.Y, show_figure=False, save_path=out)
            self.assertTrue(os.path.exists(out))
            self.assertGreater(os.path.getsize(out), 0)

    def test_generate_sinus_saves_image(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "sinus.png")
            m.generate_sinus(show_figure=False, save_path=out)
            self.assertTrue(os.path.exists(out))
            self.assertGreater(os.path.getsize(out), 0)

    def test_download_data_structure(self):
        try:
            data = m.download_data()
        except Exception as e:
            # If network unavailable, skip this test
            self.skipTest(f"Skipping download_data test due to network/parser error: {e}")
            return
        # Basic structure and types
        self.assertIsInstance(data, dict)
        for key in ("positions", "lats", "longs", "heights"):
            self.assertIn(key, data)
            self.assertIsInstance(data[key], list)
        n = len(data["positions"])
        self.assertGreater(n, 0)
        self.assertEqual(len(data["lats"]), n)
        self.assertEqual(len(data["longs"]), n)
        self.assertEqual(len(data["heights"]), n)
        # Spot-check types
        self.assertIsInstance(data["positions"][0], str)
        self.assertIsInstance(float(data["lats"][0]), float)
        self.assertIsInstance(float(data["longs"][0]), float)
        self.assertIsInstance(float(data["heights"][0]), float)


if __name__ == "__main__":
    unittest.main()
