//////////////////////////////////////////////////////////////////////////////
// breezedetectwidget.h
// Note: this class is a stripped down version of
// /kdebase/workspace/kwin/kcmkwin/kwinrules/detectwidget.h
// SPDX-FileCopyrightText: 2004 Lubos Lunak <l.lunak@kde.org>

// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QObject>
#include <QVariantMap>

namespace Breeze
{

class DetectDialog : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit DetectDialog(QObject *parent = nullptr);

    //* read window properties or select one from mouse grab
    void detect();

    //* window properties
    QVariantMap properties() const;

Q_SIGNALS:
    void detectionDone(bool);

private:
    //* properties
    QVariantMap m_properties;
};

} // namespace
