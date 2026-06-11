#ifndef DECODE_LABEL_H
#define DECODE_LABEL_H

#include <QString>
#include <QtGlobal>

// One decoded-callsign label drawn on top of the WideGraph waterfall
// at its audio-offset x-position. WideGraph maintains a list of these
// (refreshed each time mainwindow taps the decode-append path); CPlotter
// reads the list via setDecodeLabels() and overlays them in paintEvent.
//
// Lives in its own header so widegraph.h and plotter.h can both include
// it without creating a circular dependency between the two larger
// headers.
struct DecodeLabel {
    double  freq_khz;       // audio offset (kHz), straight from the decode line
    QString callsign;       // sender's call extracted from the message field
    qint64  last_seen_ms;   // wall-clock of most recent fresh decode
    int     hits;           // for tie-breaking when stacking

    // C++11 explicit ctor (gnu++11 mode in this build doesn't accept
    // QList::append({...}) brace-enclosed init lists).
    DecodeLabel(double f, const QString& c, qint64 t, int h)
        : freq_khz(f), callsign(c), last_seen_ms(t), hits(h) {}
    DecodeLabel() : freq_khz(0), last_seen_ms(0), hits(0) {}
};

#endif // DECODE_LABEL_H
