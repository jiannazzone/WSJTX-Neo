# WSJT-X Neo — Architecture Seam Audit

**Purpose:** map the frontend/backend entanglement in `MainWindow` *before* any
refactoring, so every later decision is informed. No code changes are proposed
here — this is a survey of where the natural cut-lines are.

**Scope of the god object:**
- `widgets/mainwindow.cpp` — 17,956 lines
- `widgets/mainwindow.h` — 1,094 lines (~250 member variables, ~200 methods/slots)
- `widgets/mainwindow.ui` — 6,485 lines, 236 named widgets, 166 menu/toolbar actions

---

## TL;DR — the good news and the bad news

**Good news (the "backend" is already partly separated).** WSJT-X already runs its
heaviest subsystems behind hard boundaries that `MainWindow` only *orchestrates*:

| Subsystem | Boundary that already exists | Glue left in MainWindow |
|---|---|---|
| **Decoder (Fortran/jt9)** | separate **process** — IPC via `QSharedMemory` (`mem_jt9`) + stdout pipes (`proc_jt9`, `p1`–`p4`) | ~59 mentions |
| **Audio I/O** | separate **thread** (`m_audioThread`) — async via `Q_SIGNAL`/slots | ~95 mentions |
| **Rig control (Hamlib)** | wrapped by `Transceiver` / `Configuration`, own thread | ~194 mentions |
| **UDP protocol** | `Network/MessageClient` class | ~66 mentions |

These are exactly the off-limits areas in the project brief — and they're already
the *least* entangled. We do not need to touch them to separate frontend/backend.

**Bad news (the real entanglement).** `MainWindow` is simultaneously the **view**,
the **application-state owner**, and the **orchestrator**. The mess isn't backend
code living in the UI file — it's that there is **no model layer**: ~250 state
members live directly on the window, and backend logic reads/writes widgets inline
(`ui->dxCallEntry->text()` … compute … `ui->tx1->setText(...)`). That inline
widget I/O is what must be inverted to extract anything.

---

## 1. State inventory (member variables, by category)

`MainWindow` holds ~250 data members. Grouped by what they actually belong to:

### A. Application / radio state — *no current home; belongs in a model*
- **Frequency/band:** `m_freqNominal`, `m_freqTxNominal`, `m_freqNominalPeriod`,
  `m_lastDialFreq`, `m_currentBand`, `m_lastBand`, `m_lastMonitoredFrequency`,
  `m_XIT`, `m_bandEdited`, `m_splitMode`, `m_bSimplex`
- **Mode/submode:** `m_mode`, `m_modeTx`, `m_nSubMode`, `m_TRperiod`, `m_nsps`,
  `m_specOp`, `m_bFastMode`, `m_bFast9`
- **Identity/QSO partner:** `m_baseCall`, `m_hisCall`, `m_hisGrid`, `m_deCall`,
  `m_deGrid`, `m_opCall`, `m_bMyCallStd`, `m_bHisCallStd`

### B. QSO state machine — *pure backend logic, currently inline*
- `m_QSOProgress` (the `CALLING…SIGNOFF` enum), `m_ntx`, `m_nTx73`, `m_txFirst`,
  `m_auto`, `m_currentMessage*`, `m_lastMessage*`, `m_rptSent`, `m_rptRcvd`,
  `m_xSent`, `m_xRcvd`, `m_send_RR73`, `m_sentFirst73`, the `m_dateTime*` QSO timestamps

### C. Fox/Hound DXpedition — *a sub-application embedded in the window*
- Structs `FoxQSO`, `FixupQSO`; maps `m_foxQSO`, `m_loggedByFox`, `m_fixupQSO`,
  `m_annotated_callsigns`; queues `m_houndQueue`, `m_foxQSOinProgress`,
  `m_foxRateQueue`; ~20 `m_fox*`/`m_nFox*`/`m_tFox*` scalars, `m_Nlist`, `m_Nslots`,
  `m_maxStrikes`, `m_houndCallers`, arrays `m_ready2call[50]`, `m_callers[50]`
- **735 mentions in the .cpp — the single largest cohesive blob.**

### D. Decode/contest bookkeeping — *backend models*
- Structs `ActiveCall`/`m_activeCall`, `EMECall`/`m_EMECall`, `RecentCall`/`m_recentCall`,
  `ARRL_logged`/`m_arrl_log`; `m_EMEworked`; `m_pfx`/`m_sfx` prefix/suffix sets;
  scoring (`m_points`, `m_score`, `m_maxPoints`); `m_nDecodes`, `m_nWSPRdecodes`

### E. Owned subsystem handles — *already separated; MainWindow just holds the pointer*
- Audio: `m_detector`, `m_soundInput`, `m_soundOutput`, `m_modulator`, `m_audioThread`
- Decoder IPC: `proc_jt9`, `p1`–`p4`, `mem_jt9`
- Network: `m_messageClient`, `m_psk_Reporter`, `m_cloudlog`, `wsprNet`, `Eqsl`,
  `m_network_manager`
- Config/logging: `m_config`, `m_settings`, `m_multi_settings`, `m_logBook`
- Child windows: `m_wideGraph`, `m_echoGraph`, `m_fastGraph`, `m_astroWidget`,
  `m_logDlg`, `m_msgAvgWidget`, `m_ActiveStationsWidget`, `m_foxLogWindow`,
  `m_contestLogWindow`, `m_colorHighlighting`, `m_QSYMessage*`, `m_qsymonitor*`

### F. View-only state — *belongs with the frontend*
- Status-bar labels (`tx_status_label`, `config_label`, `mode_label`,
  `last_tx_label`, `auto_tx_label`, `band_hopping_label`, `ndecodes_label`,
  `watchdog_label`, `progressBar`)
- `m_useDarkStyle`, `m_palette`, `ui` itself, geometry blobs

### G. Timers / async plumbing — *split by owner*
- ~14 `QTimer`s (`m_guiTimer`, `stopWRTimer`, `ptt0/1Timer`, `tuneATU_Timer`,
  `TxAgainTimer`, `minuteTimer`, …) — each binds to either backend sequencing or UI.
- `QFuture*` watchers for WAV save/load.

> **Rule of thumb that falls out of this:** categories **A–D** (~60% of members) are
> backend state with no model to live in. **E** is already-separated infrastructure.
> **F** is the only genuinely frontend-owned state. The refactor is essentially
> "give A–D a home outside the window."

---

## 2. Subsystem call-graph (what MainWindow talks to)

```
                         ┌─────────────────────────────────────────┐
                         │              MainWindow                  │
                         │   (view + state owner + orchestrator)    │
                         └─────────────────────────────────────────┘
        ┌──────────────┬───────────┬───────────┬───────────┬──────────────┐
        ▼              ▼           ▼           ▼           ▼              ▼
  ┌───────────┐  ┌──────────┐ ┌─────────┐ ┌────────┐ ┌──────────┐ ┌────────────┐
  │  Audio    │  │ Decoder  │ │  Rig    │ │  UDP / │ │ Logging  │ │  Display   │
  │  thread   │  │ process  │ │ Hamlib  │ │ Net    │ │ logbook  │ │  children  │
  │           │  │  (jt9)   │ │         │ │        │ │          │ │            │
  │ Sound In/ │  │ shared   │ │Transcvr │ │ Msg-   │ │ LogBook  │ │ WideGraph  │
  │ Out,      │  │ mem +    │ │ via     │ │ Client │ │ FoxLog   │ │ Echo/Fast  │
  │ Modulator,│  │ stdout   │ │ Config  │ │ PSKRep │ │ Cabrillo │ │ Astro,     │
  │ Detector  │  │ pipes    │ │         │ │ Cloud- │ │          │ │ Active-    │
  │           │  │          │ │         │ │ log,   │ │          │ │ Stations…  │
  │ async via │  │ readFrom │ │ handle_ │ │ eQSL,  │ │          │ │            │
  │ Q_SIGNALs │  │ Stdout   │ │ transc- │ │ wsprNet│ │          │ │            │
  │           │  │ to_jt9   │ │ eiver_* │ │        │ │          │ │            │
  └───────────┘  └──────────┘ └─────────┘ └────────┘ └──────────┘ └────────────┘
   ~95 glue       ~59 glue     ~194 glue   ~66 glue    (LogQSO     236 widgets,
   ALREADY        ALREADY      ALREADY     ALREADY      dialog)     166 actions
   THREADED       PROCESS      THREADED    CLASSED                  in mainwindow.ui

  *** OFF-LIMITS per brief — and already the least entangled. ***
```

The orchestration logic that lives *only* in MainWindow (no existing boundary):

```
  QSO State Machine ── m_QSOProgress / genStdMsgs / setTxMsg / auto_sequence / processMessage   (~260)
  Fox/Hound DXpedition ── foxRxSequencer / foxTxSequencer / houndCallers / sortHoundCalls       (~735)
  Mode controllers ──  WSPR (~201) · Fast/MSK144 (~450) · Echo (~166) · FreqCal · Contest (~114)
  Band/frequency mgmt ── setRig / band_changed / displayDialFrequency / setXIT
```

---

## 3. Proposed module boundaries

The target is a thin(ner) `MainWindow` (view) over a set of backend modules. None
of these require touching the off-limits subsystems — they sit *between* MainWindow
and the already-separated infrastructure.

| Proposed module | Absorbs (state + logic) | Talks to |
|---|---|---|
| **`AppState` / `RadioModel`** | category A: freq/band/mode/identity | everyone reads it; emits `changed()` signals |
| **`QsoStateModel`** | category B: the QSO state machine + message generation (`genStdMsgs`, `setTxMsg`, `auto_sequence`) | AppState, MessageClient |
| **`FoxHoundController`** | category C: the entire Fox/Hound DXpedition sub-app | QsoStateModel, FoxLog, decoder feed |
| **`ModeController` (per mode)** | WSPR / Fast-MSK144 / Echo / FreqCal strategies | AppState, decoder, audio signals |
| **`DecodeModel`** | category D: active/recent/EME call tables, scoring | decoder output, ActiveStations view |
| **`StatusPresenter`** | category F status-bar labels + `updateStatusBar` | reads models, writes labels |
| **`MainWindow` (view, shrunk)** | `ui`, child-window mgmt, wiring | binds widgets ↔ models via signals/slots |

The boundary mechanism is the one the codebase already uses everywhere: **Qt
signals/slots**. Models emit `xChanged`; the view subscribes and updates widgets;
widget edits call model setters. This is also what makes the off-limits subsystems
safe — they already communicate this way.

---

## 4. Ranked extraction candidates (easy → hard)

Ranked by **self-containment** (how few cross-cutting reads/writes) × **payoff**.

| # | Candidate | Size | Why this order | Risk |
|---|---|---|---|---|
| 1 | **StatusPresenter** (status-bar labels) | small | Labels are already discrete `QLabel` members (cat. F); `updateStatusBar()`/`createStatusBar()` are nearly standalone | **Low** — pure view code |
| 2 | **Child-window manager** | small | The `m_*Graph`/`m_*Window` pointers + their show/hide actions are loosely coupled | **Low** |
| 3 | **Echo-mode controller** | ~166 | Self-contained feature with its own graph (`EchoGraph`), own state, own UI page | **Low-Med** |
| 4 | **DecodeModel** (call tables/scoring) | cat. D | Data structures with clear owners; ActiveStations already consumes them | **Med** — feeds several views |
| 5 | **WSPR controller** | ~201 | Mode-scoped; own band-hopping, own upload path (`wsprNet`) | **Med** |
| 6 | **Fast/MSK144 controller** | ~450 | Mode-scoped but shares the fast decode path (`fastSink`, `FastGraph`) | **Med** |
| 7 | **AppState / RadioModel** | cat. A | High value (everything depends on it) but *touches everything* → do after the easy wins prove the pattern | **Med-High** |
| 8 | **QsoStateModel** | ~260 | The keystone; the state machine reads/writes many widgets inline | **High** |
| 9 | **FoxHoundController** | ~735 | Biggest payoff and biggest tangle; depends on QsoStateModel + FoxLog being extracted first | **High** |

**Recommended sequencing:** prove the signals/slots pattern on **#1–#3** (low blast
radius, immediately shrinks the window and de-risks the approach), then do the model
layer **#4, #7**, then the mode controllers **#5, #6**, and only attempt the
keystone pair **#8 → #9** once the models they depend on exist.

---

## 5. Risk notes per boundary

- **Inline widget I/O is the core hazard.** Backend logic reads widget values
  directly (`ui->dxCallEntry->text()`, `ui->sbTR->value()`) and writes results back
  (`ui->tx1->setText(...)`). Every extraction must invert these into model
  getters/setters + signals. The HEAVY-tier widgets from
  `WIDGET_COUPLING_ANALYSIS.md` (e.g. `TxFreqSpinBox` 166 refs, `bandComboBox`
  93 refs/61 slots, `decodedTextBrowser` 116 refs) are where this is densest —
  their **object names must be preserved** during any move.
- **Don't touch the off-limits subsystems.** Audio/decoder/Hamlib/UDP are already
  behind boundaries; the refactor wraps the *glue*, not the subsystems. Resist the
  temptation to "clean up" the IPC.
- **`MultiSettings` / `readSettings`/`writeSettings` are cross-cutting.** Almost
  every member is persisted. When state moves into a model, its load/save must move
  with it or the settings round-trip breaks silently.
- **Thread affinity.** Audio lives on `m_audioThread`; the decoder is a process.
  Any model that emits to those must respect the existing queued-connection
  patterns — don't introduce direct cross-thread calls.
- **The QSO state machine is timing-coupled.** `m_QSOProgress` transitions are
  driven by `guiUpdate()` on a timer and by decode arrivals; extracting it means
  preserving that tick/event ordering exactly, or auto-sequencing behavior changes.
- **Fox/Hound shares state with the QSO model and FoxLog.** It can't be cleanly
  lifted until #7/#8 give it models to depend on — attempting it first would just
  relocate the tangle.

---

## 6. What this enables for the UI/UX overhaul

Once even #1–#3 are extracted, the view shrinks and pure-styling work (QSS, layout
reflow, regrouping) gets safer because there's less logic interleaved with the
widget tree. The full model layer (#4–#9) is what would eventually allow a
*replaceable* frontend (e.g. a redesigned Widgets layout, or even QML) over an
unchanged backend — but that is the multi-month end-state, not a prerequisite for
visible UI improvements.

---
*Companion doc: `WIDGET_COUPLING_ANALYSIS.md` (per-widget cosmetic-vs-logic tiers).*
