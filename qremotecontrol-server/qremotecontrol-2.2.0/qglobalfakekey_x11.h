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
#ifndef QGLOBALFAKEKEY_X11_H
#define QGLOBALFAKEKEY_X11_H

#include <QObject>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xmd.h>
#include <X11/extensions/XTest.h>

class QGlobalFakeKeyPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QGlobalFakeKeyPrivate(QObject *parent = 0);
    ~QGlobalFakeKeyPrivate();
    void sendNativeKey(Qt::Key key, bool down);
    void sendNativeKeyModifiers(Qt::KeyboardModifiers modifiers, bool down);
    void sendNativeButton(Qt::MouseButton button, bool down);
    void sendNativeScroll(int direction, int delta, double acceleration);

private:
    Display* display;

    quint32 nativeKeycode(Qt::Key key) const;

signals:

public slots:

};

#endif // QGLOBALFAKEKEY_X11_H
