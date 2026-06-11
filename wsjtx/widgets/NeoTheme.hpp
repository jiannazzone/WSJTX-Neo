// -*- Mode: C++ -*-
#ifndef NEO_THEME_HPP
#define NEO_THEME_HPP

//
// WSJT-X Neo — application theme.
//
// Slice 1 of the UI/UX overhaul: a single, cross-platform design language.
//
// The look is built on Qt's Fusion style (which renders identically on Linux,
// macOS and Windows) plus a shared palette and a refining stylesheet, so the
// application no longer inherits three different native appearances. See
// DESIGN_DIRECTION.md for the larger plan.
//
// This is intentionally self-contained (header-only, applied once from main)
// so it touches no logic and no build wiring. Colour-coded readouts that the
// application sets through inline stylesheets (frequency display, status
// labels, mode buttons) keep their own colours; later slices address those.
//

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

namespace NeoTheme
{
  // ---- design tokens (light) -------------------------------------------
  namespace tokens
  {
    inline QColor window      () { return QColor (0xf4, 0xf5, 0xf7); } // app surface
    inline QColor surface     () { return QColor (0xff, 0xff, 0xff); } // raised / inputs
    inline QColor surfaceAlt  () { return QColor (0xec, 0xee, 0xf1); } // buttons / headers
    inline QColor border      () { return QColor (0xd4, 0xd8, 0xde); }
    inline QColor borderStrong() { return QColor (0xb9, 0xc0, 0xc9); }
    inline QColor text        () { return QColor (0x1b, 0x1f, 0x24); }
    inline QColor textMuted   () { return QColor (0x6a, 0x72, 0x7d); }
    inline QColor textDisabled() { return QColor (0xa8, 0xae, 0xb6); }
    inline QColor accent      () { return QColor (0x2a, 0x6f, 0x97); } // calm steel-teal
    inline QColor accentText  () { return QColor (0xff, 0xff, 0xff); }
  }

  // ---- palette ---------------------------------------------------------
  inline QPalette palette ()
  {
    using namespace tokens;
    QPalette p;
    p.setColor (QPalette::Window,          window ());
    p.setColor (QPalette::WindowText,      text ());
    p.setColor (QPalette::Base,            surface ());
    p.setColor (QPalette::AlternateBase,   window ());
    p.setColor (QPalette::Text,            text ());
    p.setColor (QPalette::Button,          surfaceAlt ());
    p.setColor (QPalette::ButtonText,      text ());
    p.setColor (QPalette::ToolTipBase,     surface ());
    p.setColor (QPalette::ToolTipText,     text ());
    p.setColor (QPalette::PlaceholderText, textMuted ());
    p.setColor (QPalette::Highlight,       accent ());
    p.setColor (QPalette::HighlightedText, accentText ());
    p.setColor (QPalette::Link,            accent ());

    p.setColor (QPalette::Disabled, QPalette::Text,       textDisabled ());
    p.setColor (QPalette::Disabled, QPalette::ButtonText, textDisabled ());
    p.setColor (QPalette::Disabled, QPalette::WindowText, textDisabled ());
    return p;
  }

  // ---- refining stylesheet --------------------------------------------
  inline QString styleSheet ()
  {
    return QStringLiteral (R"QSS(
* { outline: 0; }

QToolTip {
  background: #ffffff;
  color: #1b1f24;
  border: 1px solid #b9c0c9;
  border-radius: 4px;
  padding: 4px 6px;
}

/* Buttons -------------------------------------------------------------- */
QPushButton {
  background: #ffffff;
  border: 1px solid #c4cad2;
  border-radius: 5px;
  padding: 4px 12px;
  min-height: 20px;
}
QPushButton:hover   { background: #f0f3f6; border-color: #2a6f97; }
QPushButton:pressed { background: #e2e8ee; }
QPushButton:checked {
  background: #2a6f97; color: #ffffff; border-color: #245f82;
}
QPushButton:disabled { color: #a8aeb6; background: #f4f5f7; border-color: #dde1e6; }

/* Text inputs / spin / combo ------------------------------------------ */
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit {
  background: #ffffff;
  border: 1px solid #c4cad2;
  border-radius: 5px;
  padding: 3px 6px;
  selection-background-color: #2a6f97;
  selection-color: #ffffff;
}
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus,
QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus {
  border: 1px solid #2a6f97;
}
QComboBox::drop-down { border: 0; width: 18px; }

/* Group boxes ---------------------------------------------------------- */
QGroupBox {
  border: 1px solid #d4d8de;
  border-radius: 6px;
  margin-top: 14px;
  padding: 8px;
}
QGroupBox::title {
  subcontrol-origin: margin;
  subcontrol-position: top left;
  left: 8px;
  padding: 0 4px;
  color: #6a727d;
}

/* Checks / radios ------------------------------------------------------ */
QCheckBox, QRadioButton { spacing: 6px; padding: 1px 0; }

/* Tabs ----------------------------------------------------------------- */
QTabWidget::pane { border: 1px solid #d4d8de; border-radius: 6px; top: -1px; }
QTabBar::tab {
  background: #eceef1;
  border: 1px solid #d4d8de;
  padding: 5px 12px;
  border-top-left-radius: 5px;
  border-top-right-radius: 5px;
}
QTabBar::tab:selected   { background: #ffffff; border-bottom-color: #ffffff; }
QTabBar::tab:!selected   { margin-top: 2px; color: #6a727d; }

/* Decode tables / lists ------------------------------------------------ */
QHeaderView::section {
  background: #eceef1;
  color: #4a525c;
  border: 0;
  border-right: 1px solid #d4d8de;
  border-bottom: 1px solid #d4d8de;
  padding: 4px 6px;
}
QTableView, QTreeView, QListView {
  border: 1px solid #d4d8de;
  border-radius: 6px;
  background: #ffffff;
}

/* Menus ---------------------------------------------------------------- */
QMenuBar { background: #f4f5f7; }
QMenuBar::item:selected { background: #e2e8ee; }
QMenu { background: #ffffff; border: 1px solid #c4cad2; }
QMenu::item:selected { background: #2a6f97; color: #ffffff; }

/* Scrollbars ----------------------------------------------------------- */
QScrollBar:vertical   { background: #f4f5f7; width: 12px; margin: 0; }
QScrollBar:horizontal { background: #f4f5f7; height: 12px; margin: 0; }
QScrollBar::handle {
  background: #c4cad2; border-radius: 5px; min-height: 24px; min-width: 24px;
}
QScrollBar::handle:hover { background: #b9c0c9; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }

/* Status / progress ---------------------------------------------------- */
QStatusBar { background: #f4f5f7; }
QProgressBar {
  border: 1px solid #d4d8de; border-radius: 5px; text-align: center;
  background: #ffffff;
}
QProgressBar::chunk { background: #2a6f97; border-radius: 4px; }

/* Splitter handle between decode panes --------------------------------- */
QSplitter::handle { background: #d4d8de; }
QSplitter::handle:horizontal { width: 3px; }
QSplitter::handle:vertical   { height: 3px; }
)QSS");
  }

  // ---- entry point -----------------------------------------------------
  // Apply once, early in main(), before any baseline stylesheet snapshot.
  inline void apply (QApplication & app)
  {
    if (auto * fusion = QStyleFactory::create (QStringLiteral ("Fusion")))
      {
        app.setStyle (fusion);
      }
    app.setPalette (palette ());
    app.setStyleSheet (styleSheet ());
  }
}

#endif // NEO_THEME_HPP
