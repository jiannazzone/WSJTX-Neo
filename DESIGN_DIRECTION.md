# WSJT-X Neo — UI/UX Design Direction

The goal of this fork is a UI/UX overhaul that **feels nice and is consistent
across Linux, macOS, and Windows**, without touching signal processing, decoding,
the Fortran DSP, rig control, or the UDP protocol.

## Current-state pain points

Grounded in the running app (native Qt rendering on macOS):

1. **Dated, platform-native look.** The app inherits each OS's native widget style,
   so it looks different on every platform and has no identity of its own.
2. **Flat hierarchy.** The two large decode tables dominate the window while the
   controls you actually interact with are crammed into the bottom third at uniform
   visual weight — the eye has no anchor.
3. **Density without grouping.** DX entry, frequency spinners, sequencing toggles,
   and the Tx1–6 message strip sit shoulder-to-shoulder with no sectioning.
4. **Incoherent color.** LCD-yellow readouts, a red "OOB" chip, a green mode
   button, a pink status bar — signals accumulated over time with no system.
5. **Cramped sizing.** Labels clip ("Lookup" → "ɔokı"). Default fonts, no spacing
   rhythm.
6. **A 41-checkbox wall.** Most main-surface toggles (BP, SWL, Sh, Measure, Fast,
   Menus, …) are set-once-and-forget yet permanently consume attention.

## Design principles

- **One design language, identical on every OS.** Stop inheriting native styles.
  Base the app on Qt's **Fusion** style plus a single custom **QSS theme**
  (light + dark). Fusion renders consistently on all three platforms; we ship our
  own look instead of three different native ones.
- **Clear hierarchy.** Decodes are primary (give them room + better table styling);
  the QSO control cluster is secondary but well organized.
- **Group, don't crowd.** Collect related controls into labeled sections
  (Station/DX · Frequency · Sequencing · Messages) with a consistent spacing rhythm.
- **Restrained color with meaning.** Neutral grey base + one accent (calm
  blue/teal); semantic color used sparingly (green = active/monitoring, amber =
  warning, red = error/out-of-band).
- **Progressive disclosure.** Move rarely-used toggles off the main surface into
  settings/overflow.
- **Preserve all logic.** Everything is QSS + layout reflow that keeps widget
  object names intact, so the tightly-bound logic (see
  `WIDGET_COUPLING_ANALYSIS.md`) is untouched and there is no DSP/logic risk.

## Visual language (tokens)

- **Palette:** neutral greys for surfaces/borders; one accent for selection/focus;
  semantic green/amber/red. Light and dark variants share the same accent.
- **Typography:** one UI font with a type scale; a tabular/mono font *only* for the
  decode tables and the frequency readout.
- **Spacing:** 4 / 8 px rhythm; section padding and separators instead of a flat
  field of widgets.
- **Components:** consistent button heights with hover/press states; mode column
  and band row become segmented controls; restyled inputs, tables, and readouts.

## Roadmap (lowest-risk, most-visible first)

1. **Theming foundation** — switch to Fusion + load a QSS stylesheet (light/dark).
   One global change, instantly modernizes everything, inherently cross-platform,
   zero logic risk. *(Starting here.)*
2. **Palette + typography** — apply the tokens.
3. **Component restyle** — buttons, inputs, tables, readouts, mode/band segmented
   controls.
4. **IA / decluttering** — section the control cluster; progressive disclosure for
   rare toggles.
5. **Targeted refactors** — extract model seams only where the new IA needs them
   (see `ARCHITECTURE_SEAMS.md`).

## Constraints

- No changes to `lib/` (Fortran DSP), `Audio/`, Hamlib/rig control, or the UDP
  protocol.
- Preserve widget object names referenced from `mainwindow.cpp` unless the cpp is
  updated in tandem.
- Theme must look correct in both light and dark and on all three platforms.
