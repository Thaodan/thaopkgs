/**************************************************************************
**
** This file is part of QRemoteControl-Server.
**
** QRemoteControl-Server is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** QRemoteControl-Server is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with PhyxCalc.  If not, see <http://www.gnu.org/licenses/>.
**
***************************************************************************/
#ifndef QGLOBALFAKEKEY_H
#define QGLOBALFAKEKEY_H

#include <QObject>
#include <QKeySequence>
#include <QDebug>

#ifdef Q_WS_X11
#include "qglobalfakekey_x11.h"
#endif
#ifdef Q_WS_WIN
#include "qglobalfakekey_win.h"
#endif

class QGlobalFakeKey : public QObject
{
    Q_OBJECT
public:
    explicit QGlobalFakeKey(QObject *parent = 0);

    static void sendKey(Qt::Key key, bool down);
    static void sendKeyModifiers(Qt::KeyboardModifiers modifiers, bool down);
    static void sendButton(Qt::MouseButton button, bool down);
    static void sendScroll(int direction, int delta, double acceleration);

signals:

public slots:
};

#endif // QGLOBALFAKEKEY_H
