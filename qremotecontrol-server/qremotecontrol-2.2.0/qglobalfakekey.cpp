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
#include "qglobalfakekey.h"

QGlobalFakeKey::QGlobalFakeKey(QObject *parent) :
    QObject(parent)
{
}

void QGlobalFakeKey::sendKey(Qt::Key key, bool down)
{
    QGlobalFakeKeyPrivate globalFakeKey;
    globalFakeKey.sendNativeKey(key, down);
}
void QGlobalFakeKey::sendKeyModifiers(Qt::KeyboardModifiers modifiers, bool down)
{
    QGlobalFakeKeyPrivate globalFakeKey;
    globalFakeKey.sendNativeKeyModifiers(modifiers, down);
}

void QGlobalFakeKey::sendButton(Qt::MouseButton button, bool down)
{
    QGlobalFakeKeyPrivate globalFakeKey;
    globalFakeKey.sendNativeButton(button, down);
}

void QGlobalFakeKey::sendScroll(int direction, int delta, double acceleration)
{
    QGlobalFakeKeyPrivate globalFakeKey;
    globalFakeKey.sendNativeScroll(direction, delta, acceleration);
}
