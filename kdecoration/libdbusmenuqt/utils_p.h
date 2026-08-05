/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2010 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

class QString;

/**
 * Swap mnemonic char: Qt uses '&', while dbusmenu uses '_'
 */
QString swapMnemonicChar(const QString &in, char src, char dst);
