/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2010 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "utils_p.h"

// Qt
#include <QString>

// STL
#include <algorithm>
#include <iterator>

QString swapMnemonicChar(const QString &in, char src, char dst)
{
    if (in.isEmpty()) {
        return in;
    }

    const QChar qsrc = QLatin1Char(src);
    const QChar qdst = QLatin1Char(dst);

    // Optimization: find first character that needs processing
    const QChar *data = in.constData();
    const QChar *const end = data + in.length();
    const QChar *ptr = std::find_if(data, end, [qsrc, qdst](QChar c) {
        return c == qsrc || c == qdst;
    });

    if (ptr == end) {
        return in;
    }

    QString out;
    out.reserve(in.length() + 4);
    out.append(data, std::distance(data, ptr));

    bool mnemonicFound = false;

    for (; ptr < end; ++ptr) {
        const QChar ch = *ptr;
        if (ch == qsrc) {
            if (ptr + 1 == end) {
                // 'src' at the end of string, keep it
                out.append(qsrc);
            } else if (*(ptr + 1) == qsrc) {
                // A real 'src'
                out.append(qsrc);
                ++ptr;
            } else {
                bool isMnemonic = !mnemonicFound;

                // Heuristic: if followed by a digit and preceded by an alphanumeric char,
                // it's probably part of an identifier (like video_1) rather than a mnemonic.
                if (isMnemonic && (ptr + 1)->isDigit() && ptr > data && (ptr - 1)->isLetterOrNumber()) {
                    isMnemonic = false;
                }

                if (isMnemonic) {
                    mnemonicFound = true;
                    out.append(qdst);
                } else {
                    out.append(qsrc);
                }
            }
        } else if (ch == qdst) {
            // Escape 'dst'
            out.append(qdst);
            out.append(qdst);
        } else {
            out.append(ch);
        }
    }

    return out;
}
