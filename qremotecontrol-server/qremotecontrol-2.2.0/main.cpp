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
#include <QtGui/QApplication>
#include <QtSingleApplication>
#include "qremotecontrolserver.h"

int main(int argc, char *argv[])
{
    QtSingleApplication instance(argc, argv);
    if (instance.sendMessage("hello"))
    {
        qWarning("QRemoteControl-Server is already running");
        return 0;
    }

    QRemoteControlServer w;
    QApplication::setQuitOnLastWindowClosed(false);

    instance.setActivationWindow(&w);

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
                     &w, SLOT(applicationStarted(QString)));

    return instance.exec();
}
