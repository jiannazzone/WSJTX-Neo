# WSJT-X Neo

A user-interface and user-experience overhaul of [WSJT-X](https://wsjt.sourceforge.io/),
the amateur-radio weak-signal digital-modes application.

WSJT-X Neo is a fork of **WSJT-X Improved** (itself based on WSJT-X by Joseph
Taylor, K1JT, and the WSJT Development Group). The goal of this fork is strictly a
modernized UI/UX — the signal processing, decoders/encoders, the Fortran DSP, rig
control (Hamlib), and the UDP message protocol are inherited unchanged from
upstream.

## Status

Early development. The Qt6 source tree builds and runs natively on macOS; Linux and
Windows builds are validated through continuous integration.

## Building

The application source lives in [`wsjtx/`](wsjtx/) and uses CMake + Ninja with Qt6.

- **macOS:** see [`BUILD_MACOS.md`](BUILD_MACOS.md) for the full, verified recipe.
- **Linux / Windows:** see the CI workflow in
  [`.github/workflows/build.yml`](.github/workflows/build.yml) for the exact
  dependency lists and build steps used on each platform.

## Project notes

- [`ARCHITECTURE_SEAMS.md`](ARCHITECTURE_SEAMS.md) — analysis of the frontend/backend
  boundaries in `MainWindow` and ranked refactoring candidates.
- [`WIDGET_COUPLING_ANALYSIS.md`](WIDGET_COUPLING_ANALYSIS.md) — per-widget map of
  which UI elements are cosmetic versus tightly bound to application logic.

## License

WSJT-X Neo is free software licensed under the **GNU General Public License,
version 3** (GPLv3), in accordance with the license of the upstream WSJT-X sources.
See [`LICENSE`](LICENSE) for the full text. The original upstream license text is
also retained at [`wsjtx/COPYING`](wsjtx/COPYING).

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
