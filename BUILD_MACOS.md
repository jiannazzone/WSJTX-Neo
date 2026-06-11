# Building WSJT-X Neo on macOS (Apple Silicon)

Verified working: macOS 26 / arm64, 2026-06-11. Produces a runnable
`wsjtx.app` + `jt9` decoder.

## Toolchain (Homebrew)

```sh
brew install cmake qt gcc libomp fftw boost hamlib libusb ninja
```

- `qt` → Qt 6.11.1 (Widgets, SerialPort, Multimedia, PrintSupport, Sql, WebSockets, LinguistTools)
- `gcc` → provides `gfortran` (Fortran DSP); AppleClang builds the C++/Qt side
- `fftw` provides single-precision (`libfftw3f`) + threads, required
- `hamlib`, `boost` (log/log_setup), `libusb` — all found via the `/opt/homebrew` prefix

## Configure + build (out-of-source, Ninja)

```sh
cd wsjtx-neo
cmake -G Ninja -S wsjtx -B build \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/homebrew" \
  -DCMAKE_Fortran_COMPILER=/opt/homebrew/bin/gfortran \
  -DCMAKE_BUILD_TYPE=Release \
  -DWSJT_GENERATE_DOCS=OFF \
  -DWSJT_SKIP_MANPAGES=ON
ninja -C build
# result: build/wsjtx.app/Contents/MacOS/wsjtx  (+ build/jt9)
```

### Make the dev build runnable (helper decoders)

`ninja` builds the decoder helpers (`jt9`, `wsprd`, freq-cal tools, the `*sim`/
`*code` utilities) at the **build root**, but `wsjtx` spawns them from inside its
own bundle (`wsjtx.app/Contents/MacOS/`). Without this step you get
*"Subprocess error … jt9 … No such file or directory"* the moment decoding starts.
A real `macdeployqt`/install copies them in; for the dev loop, symlink them:

```sh
cd build/wsjtx.app/Contents/MacOS
for f in ../../../*; do b=$(basename "$f"); \
  [ -f "$f" ] && [ -x "$f" ] && [ "$b" != wsjtx ] && file "$f" | grep -q Mach-O \
  && ln -sf "../../../$b" "$b"; done
```

Re-run only if new helpers appear; existing symlinks survive incremental rebuilds.

Run: `open build/wsjtx.app` (first launch opens Settings — needs a callsign).

## Notes / gotchas

- **Deployment target fix (required).** `CMakeLists.txt:13` hardcoded
  `CMAKE_OSX_DEPLOYMENT_TARGET 10.12` (a Qt 5.12-era value). Qt 6 headers use
  `std::filesystem` (needs 10.15+) and Qt 6.11 needs macOS 12, so 10.12 fails to
  compile. **Changed to `12.0`** in `CMakeLists.txt`. If reconfiguring an existing
  build dir, the old value is cached — pass `-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0`
  once or delete `build/CMakeCache.txt`.
- **`ld` "built for newer version" warnings are expected and harmless locally.**
  Homebrew's Qt dylibs are built for macOS 14 and gfortran's runtime for the host
  (26). We target 12.0, so the linker warns. The app still runs fine on *this*
  machine. For a *distributable* build the effective floor is whatever the bundled
  dylibs require — handled later by `macdeployqt`/packaging (#3), not the dev build.
- **Docs/manpages disabled** (`WSJT_GENERATE_DOCS=OFF`, `WSJT_SKIP_MANPAGES=ON`)
  to avoid the `asciidoc` dependency. Re-enable for release packaging.
- **Portaudio is Windows-only** here (gated by `if(WIN32)`); not needed on macOS.
- `qmap` subdir builds unconditionally and compiled fine. `map65` is Windows-only.
- Build is ~1,897 targets; full rebuild is the slow part (Fortran + the
  17.9k-line `mainwindow.cpp`). Incremental rebuilds after UI edits are fast.

## Toolchain mapping for CI (#2, cross-platform)

Same GCC/Clang + gfortran toolchain works on all three targets (Windows uses
MSYS2/mingw, not MSVC — Fortran rules out MSVC). A GitHub Actions matrix should
install Qt via `aqtinstall` (pin the version), gfortran, fftw, boost, hamlib, then
run the same `cmake … && ninja` + `cpack` (DragNDrop / NSIS / DEB).
