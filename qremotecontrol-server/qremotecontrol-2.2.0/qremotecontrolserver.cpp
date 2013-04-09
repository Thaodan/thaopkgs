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
#include "qremotecontrolserver.h"
#include "ui_qremotecontrolserver.h"
#include "qglobalfakekey.h"

const QString updateUrl = "https://dl.dropbox.com/u/50861330/qremotecontrol/version";

QRemoteControlServer::QRemoteControlServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QRemoteControlServer)
{
    ui->setupUi(this);

    //nasty workaround for finding the settings directory
#ifndef PORTABLE
    QSettings tmpConfig(QSettings::IniFormat, QSettings::UserScope, "qremotecontrol", "settings");
    settingsDir = QFileInfo(tmpConfig.fileName()).absolutePath() + "/";
#else
    settingsDir = QDir::currentPath() + "/settings/";
#endif
    loadSettings();

    tcpSocket = NULL;

    udpSocket = new QUdpSocket(this);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(incomingUdpData()));
    udpSocket->bind(ui->portSpin->value(),  QUdpSocket::DontShareAddress | QUdpSocket::ReuseAddressHint);

#ifdef PLAYERCONTROL
    //initializing PlayerController
    playerController = new PlayerController();
#ifdef QT_DEBUG
    qDebug() << "playercontroller activated";
#endif
#endif

#ifdef LIGHTCONTROL
    localSocket = NULL;
#endif

    //initializing the TrayIcon
    initializeTrayIcon();

    //hide unneeded GUI elements
    ui->statusLabel->setVisible(false);

    //connecting setting edits
    connect(ui->commandEdit, SIGNAL(editingFinished()), this, SLOT(saveTempChanges()));
    connect(ui->parametersEdit, SIGNAL(editingFinished()), this, SLOT(saveTempChanges()));
    connect(ui->shortcutDetector, SIGNAL(editingFinished()), this, SLOT(saveTempChanges()));

    //start IP-address update timer
    QTimer *ipTimer = new QTimer();
    connect(ipTimer, SIGNAL(timeout()),
            this, SLOT(refreshIpAddress()));
    refreshIpAddress();
    ipTimer->start(5000);

    //initialize timer for delayed modifier key release
    delayedReleaseTimer = new QTimer();
    delayedReleaseTimer->setInterval(ui->modifierTimeoutSpin->value());
    delayedReleaseTimer->setSingleShot(true);
    connect(delayedReleaseTimer, SIGNAL(timeout()), this, SLOT(incomingButtonDelayedRelease()));
    delayedReleaseModifiers = Qt::NoModifier;

    ui->shortcutDetector->installEventFilter(this);

    initializeButtonConfigurator();
    buttonSettingList = defaultButtonSettings();

    //check only for updates on Windows
#ifdef Q_WS_WIN
    ui->autoUpdateCheck->setVisible(true);
    if (ui->autoUpdateCheck->isChecked())
        QTimer::singleShot(15000, this, SLOT(checkForUpdates()));
#else
    ui->autoUpdateCheck->setVisible(false);
#endif
}

QRemoteControlServer::~QRemoteControlServer()
{
    saveSettings();

    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
        tcpSocket->deleteLater();
    }
#ifdef LIGHTCONTROL
    if (localSocket) {
        localSocket->disconnectFromServer();
        localSocket->deleteLater();
    }
#endif

    delete ui;
}

void QRemoteControlServer::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QRemoteControlServer::applicationStarted(const QString &message)
{
    if (message == "hello") //if second application is started show the config window
    {
        this->show();
        this->raise();
        this->activateWindow();
        this->setFocus();
    }
}

void QRemoteControlServer::initializeTrayIcon()
{
    QMenu *menu = new QMenu(this);

#if (QT_VERSION >= 0x040800) && !defined(Q_WS_WIN)
    menu->addAction(QIcon::fromTheme("configure"),tr("&Settings"),this, SLOT(on_actionSettings_triggered()));
    menu->addAction(QIcon::fromTheme("system-help"),tr("&Help"),this, SLOT(on_actionHelp_triggered()));
    menu->addAction(QIcon::fromTheme("help-about"),tr("&About"),this, SLOT(on_actionAbout_triggered()));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("application-exit"),tr("&Exit"),qApp, SLOT(quit()));
#else
    menu->addAction(QIcon(":/settings/configure"),tr("&Settings"),this, SLOT(on_actionSettings_triggered()));
    menu->addAction(QIcon(":/settings/system-help"),tr("&Help"),this, SLOT(on_actionHelp_triggered()));
    menu->addAction(QIcon(":/settings/help-about"),tr("&About"),this, SLOT(on_actionAbout_triggered()));
#ifdef Q_WS_WIN
    menu->addAction(QIcon(),tr("&Check for updates"),this, SLOT(checkForUpdates()));
#endif
    menu->addSeparator();
    menu->addAction(QIcon(":/settings/application-exit"),tr("&Exit"),qApp, SLOT(quit()));
#endif

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(menu);
    setIcon("red");
    trayIcon->setVisible(true);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->setVisible(trayIconVisible);
}

void QRemoteControlServer::loadSettings()
{
    QSettings settings(settingsDir + "settings.ini", QSettings::IniFormat);

    if (settings.value("firstStart", true).toBool() == true)
    {
        HelpDialog helpDialog;
        helpDialog.exec();
    }

    settings.beginGroup("cursor");
        ui->cursorAccelerationSpin->setValue(settings.value("acceleration", 1.5).toDouble());
        ui->scrollSpeedSpin->setValue(settings.value("scrollSpeed", 0.5).toDouble());
    settings.endGroup();

    settings.beginGroup("keyboard");
        ui->modifierTimeoutSpin->setValue(settings.value("modifierTimeout", 1500).toInt());
    settings.endGroup();

    password = settings.value("password", QString()).toString();

    settings.beginGroup("network");
        ui->portSpin->setValue(settings.value("port", 5487).toInt());
    settings.endGroup();

    int size = settings.beginReadArray("userCommands");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        UserCommand userCommand;
        userCommand.name = settings.value("name").toString();
        userCommand.type = (UserCommandType)settings.value("type", -1).toInt();
        userCommand.iconPath = settings.value("iconPath", "").toString();
        switch (userCommand.type) {
        case CommandType: userCommand.command       = settings.value("command").toString();
                          userCommand.parameters    = settings.value("parameters").toString();
                          break;
        case ShortcutType: userCommand.shortcut = settings.value("shortcut").toString();
                            userCommand.key = (Qt::Key)settings.value("key").toUInt();
                            userCommand.modifiers = (Qt::KeyboardModifiers)settings.value("modifiers").toUInt();
                           break;
        }
        userCommands.append(userCommand);
    }
    settings.endArray();

    size = settings.beginReadArray("buttonSettings");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        ButtonSetting buttonSetting;
        buttonSetting.key = (Qt::Key)settings.value("key").toUInt();
        buttonSetting.modifiers = (Qt::KeyboardModifiers)settings.value("modifiers").toUInt();
        buttonSetting.text = settings.value("text").toString();
    }
    settings.endArray();

    if (size == 0)
        buttonSettingList = defaultButtonSettings();

    settings.beginGroup("general");
        trayIconVisible = settings.value("showTrayIcon", true).toBool();
        autoStartEnabled = settings.value("autoStartEnabled", false).toBool();
        ui->autoUpdateCheck->setChecked(settings.value("autoUpdateEnabled", true).toBool());
    settings.endGroup();
}

void QRemoteControlServer::saveSettings()
{
    QSettings settings(settingsDir + "settings.ini", QSettings::IniFormat);
    settings.clear();

    settings.setValue("firstStart", false);

    settings.beginGroup("cursor");
        settings.setValue("acceleration", ui->cursorAccelerationSpin->value());
        settings.setValue("scrollSpeed", ui->scrollSpeedSpin->value());
    settings.endGroup();

    settings.beginGroup("keyboard");
        settings.setValue("modifierTimeout", ui->modifierTimeoutSpin->value());
    settings.endGroup();

    settings.setValue("password", password);

    settings.beginGroup("network");
        settings.setValue("port", ui->portSpin->value());
    settings.endGroup();

    settings.beginWriteArray("userCommands");
    for (int i = 0; i < userCommands.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", userCommands.at(i).name);
        settings.setValue("type", userCommands.at(i).type);
        settings.setValue("iconPath", userCommands.at(i).iconPath);
        switch (userCommands.at(i).type) {
            case CommandType:   settings.setValue("command", userCommands.at(i).command);
                                settings.setValue("parameters", userCommands.at(i).parameters);
                                break;
            case ShortcutType:  settings.setValue("shortcut", userCommands.at(i).shortcut);
                                settings.setValue("key", (uint)userCommands.at(i).key);
                                settings.setValue("modifiers", (uint)userCommands.at(i).modifiers);
                                break;
        }
    }
    settings.endArray();

    settings.beginWriteArray("buttonSettings");
    for (int i = 0; i < buttonSettingList.size(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("key", (uint)buttonSettingList.at(i).key);
        settings.setValue("modifiers", (uint)buttonSettingList.at(i).modifiers);
        settings.setValue("text", buttonSettingList.at(i).text);
    }
    settings.endArray();

    settings.beginGroup("general");
        settings.setValue("showTrayIcon",  trayIconVisible);
        settings.setValue("autoStartEnabled", autoStartEnabled);
        settings.setValue("autoUpdateEnabled", ui->autoUpdateCheck->isChecked());
    settings.endGroup();
}

void QRemoteControlServer::incomingConnectionRequest()
{
    QByteArray datagram;
    QByteArray answerDatagram;
    QHostAddress hostAddress;
    datagram.resize(udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(datagram.data(), datagram.size(), &hostAddress);
#ifdef QT_DEBUG
        qDebug() << "incoming data:" << datagram;
#endif

    QString magicMessageConnectionRequest = "QRC:Hello";
    QString magicMessageBroadcast = "QRC:Broadcast";

    if (datagram == magicMessageBroadcast)
    {
        if (tcpSocket == NULL)
            answerDatagram = "QRC:NotConnected";
        else
            answerDatagram = "QRC:Connected";

        udpSocket->writeDatagram(answerDatagram,hostAddress, ui->portSpin->value());
#ifdef QT_DEBUG
        qDebug() << "broadcast answered:" << answerDatagram;
#endif
    }

    if ((datagram.indexOf(magicMessageConnectionRequest) == 0) && (tcpSocket == NULL)) {
#ifdef QT_DEBUG
        qDebug() << "icoming connection request";
#endif
        if (!password.isEmpty())    //need a password
        {
            QString passwordString;
            passwordString.append(password);
            //passwordString.append(hostAddress.toString());
            QByteArray passwordHash = QCryptographicHash::hash(passwordString.toUtf8(), QCryptographicHash::Sha1);
            datagram.remove(0, magicMessageConnectionRequest.length());
            if (datagram != passwordHash)
            {
#ifdef QT_DEBUG
                qDebug() << "password incorrect request neglected";
#endif
                answerDatagram = "QRC:PasswordIncorrect";
                udpSocket->writeDatagram(answerDatagram,hostAddress, ui->portSpin->value());
                return;
            }
        }

        answerDatagram = "QRC:ServerConnecting";
        udpSocket->writeDatagram(answerDatagram,hostAddress, ui->portSpin->value());

        tcpSocket = new QTcpSocket();

        connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(deleteConnection()));
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));

#ifdef QT_DEBUG
        qDebug() << "request acctepted: connecting to client";
#endif
        tcpSocket->connectToHost(hostAddress, ui->portSpin->value());
    }
}

void QRemoteControlServer::incomingData()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress hostAddress;

        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &hostAddress);

#ifdef QT_DEBUG
        qDebug() << "incoming udp: data:" << datagram;
#endif

        if (hostAddress == tcpSocket->peerAddress()) {
            QDataStream streamIn(&datagram, QIODevice::ReadOnly);
            quint8 mode1;
            streamIn >> mode1;  //read the first data byte containing data1

            switch (mode1) {
                case 1: incomingKey(datagram);
                        break;
                case 2: incomingMouse(datagram);
                        break;
                case 3: incomingAction(datagram);
                        break;
                case 5: incomingButton(datagram);
                        break;
#ifdef LIGHTCONTROL
                case 4: incomingLight(datagram);
                        break;
#endif
            }
        }
    }
}

void QRemoteControlServer::incomingUdpData()
{
    if (tcpSocket == NULL)          //if not connected treat incoming data as connection request
        incomingConnectionRequest();
    else
        incomingData();
}

void QRemoteControlServer::incomingKey(QByteArray data)
{
    quint8 mode1;
    quint32 key;
    quint32 modifiers;
    bool keyPressed;

    QDataStream streamIn(&data, QIODevice::ReadOnly);
    streamIn >> mode1;
    streamIn >> key;
    streamIn >> modifiers;
    streamIn >> keyPressed;

#ifdef PLAYERCONTROL
    int status = playerController->status().int1;

    if ((status != -1) && (keyPressed = true))
    {
        switch (key)
        {
            case Qt::Key_MediaPlay:
                playerController->playPause();
                sendAmarokData();
                break;
            case Qt::Key_MediaPrevious:
                playerController->prev();
                sendAmarokData();
                break;
            case Qt::Key_MediaNext:
                playerController->next();
                sendAmarokData();
                break;
            default:
                QGlobalFakeKey::sendKeyModifiers((Qt::KeyboardModifier)modifiers, keyPressed);
                QGlobalFakeKey::sendKey((Qt::Key)key, keyPressed);
        }
    }
    else
#endif
        QGlobalFakeKey::sendKeyModifiers((Qt::KeyboardModifier)modifiers, keyPressed);
        QGlobalFakeKey::sendKey((Qt::Key)key, keyPressed);

#ifdef QT_DEBUG
    qDebug() << "key: #" << key << ", checked =" << keyPressed;
#endif
}

void QRemoteControlServer::incomingMouse(QByteArray data)
{
    quint8 mode1;
    quint8 mode2;
    QDataStream streamIn(&data, QIODevice::ReadOnly);
    streamIn >> mode1;
    streamIn >> mode2;

    if (mode2 == 0) {
        QPoint delta;
        streamIn >> delta;

        //fixes rounding problems
        double x = delta.x();
        double y = delta.y();
        double acceleration = ui->cursorAccelerationSpin->value();

        x*=acceleration;
        y*=acceleration;

        if (QCursor::pos() != QPoint(qRound(cursorPos.x()),qRound(cursorPos.y())))  //something has changed
        {
            cursorPos = QCursor::pos();
        }

        cursorPos.setX(cursorPos.x()+x);
        cursorPos.setY(cursorPos.y()+y);

        QCursor::setPos(QPoint(qRound(cursorPos.x()),qRound(cursorPos.y())));

#ifdef QT_DEBUG
        qDebug() << QPointF(x,y);
        qDebug() << "mouse move: delta =" << delta;
        qDebug() << QCursor::pos();
#endif
    }
    else if (mode2 == 1) {
        quint8 button;
        bool    buttonPressed;
        streamIn >> button;
        streamIn >> buttonPressed;

        Qt::MouseButton mouseButton = Qt::NoButton;
        if (button == 1)
            mouseButton = Qt::LeftButton;
        else if (button == 3)
            mouseButton = Qt::RightButton;
        else if (button == 2)
#if QT_VERSION <= 0x047000
            mouseButton = Qt::MidButton;
#else
            mouseButton = Qt::MiddleButton;
#endif

        QGlobalFakeKey::sendButton(mouseButton, buttonPressed);

#ifdef QT_DEBUG
        qDebug() << "mouse button " << button << ",checked =" << buttonPressed;
#endif
    }
    else if (mode2 == 2) {
        quint8 direction;
        qint8 delta;
        streamIn >> direction;
        streamIn >> delta;

        QGlobalFakeKey::sendScroll(direction, delta, ui->scrollSpeedSpin->value());

#ifdef QT_DEBUG
        qDebug() << "mouse scroll: direction =" << direction << ",delta =" << delta;
#endif
   }
}

void QRemoteControlServer::incomingAction(QByteArray data)
{
    quint8 mode1;
    quint8 id;
    bool pressed = true;
    QDataStream streamIn(&data, QIODevice::ReadOnly);
    streamIn >> mode1;
    streamIn >> id;
    streamIn >> pressed;

    if (id > userCommands.size())
        return;

    if ((userCommands.at(id-1).type == CommandType) && pressed)
    {
        QProcess *myProcess = new QProcess();
        if (userCommands.at(id -1).parameters.isEmpty())
            myProcess->start(userCommands.at(id - 1).command);
        else
            myProcess->start(userCommands.at(id - 1).command, userCommands.at(id -1).parameters.split(" "));
    }
    else if (userCommands.at(id-1).type == ShortcutType)
    {
        sendKeyAndModifiers(userCommands.at(id-1).key, userCommands.at(id-1).modifiers, pressed);
    }

#ifdef QT_DEBUG
    qDebug() << "executing action" << id << "of" << userCommands.size() << pressed;
#endif
}

void QRemoteControlServer::incomingButton(QByteArray data)
{
    quint8 mode1;
    quint8 id;
    bool keyPressed;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers;

    QDataStream streamIn(&data, QIODevice::ReadOnly);
    streamIn >> mode1;
    streamIn >> id;
    streamIn >> keyPressed;

    key = buttonSettingList.at(id-1).key;
    modifiers = buttonSettingList.at(id-1).modifiers;

    sendKeyAndModifiers(key, modifiers, keyPressed);

#ifdef QT_DEBUG
    qDebug() << "button: #" << id << ", checked =" << keyPressed;
#endif
}

void QRemoteControlServer::incomingButtonDelayedRelease()
{
    QGlobalFakeKey::sendKeyModifiers(delayedReleaseModifiers, false);
    delayedReleaseModifiers = 0;
}

//This function is needed for delayed modifier release!
void QRemoteControlServer::sendKeyAndModifiers(Qt::Key key, Qt::KeyboardModifiers modifiers, bool keyPressed)
{
    if (keyPressed) {
        if (delayedReleaseModifiers != modifiers) {
            if (delayedReleaseModifiers != Qt::NoModifier) {
                delayedReleaseTimer->stop();
                QGlobalFakeKey::sendKeyModifiers(delayedReleaseModifiers, false);
                delayedReleaseModifiers = Qt::NoModifier;
            }
            if (modifiers != Qt::NoModifier)
                QGlobalFakeKey::sendKeyModifiers(modifiers, true);
        }
        QGlobalFakeKey::sendKey(key, true);
    } else {
        if (modifiers != Qt::NoModifier) {
            delayedReleaseModifiers = modifiers;
            delayedReleaseTimer->start();
        }
        QGlobalFakeKey::sendKey(key, false);
    }
}

void QRemoteControlServer::sendIcons()
{
    QByteArray payload;
    QDataStream dataStream(&payload, QIODevice::WriteOnly);

    for (int i = 0; i < userCommands.size(); ++i) {                   //generating the stream
        QPixmap pixmap(userCommands.at(i).iconPath);
        if (!pixmap.isNull())
            pixmap = pixmap.scaled(QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        dataStream  << (quint8)(i+1)
                    << userCommands.at(i).name
                    << pixmap;
    }

    payload.prepend(QString("1%1").arg(payload.size(), 10, 10, QChar('0')).toLocal8Bit()); //prepending the necessary data for checking

    tcpSocket->write(payload);
}

void QRemoteControlServer::sendVersion()
{
    QByteArray datagram;

    datagram.append(QString("Version:%1").arg(VERSION).toLocal8Bit());
    udpSocket->writeDatagram(datagram, tcpSocket->peerAddress(), ui->portSpin->value());
}

#ifdef LIGHTCONTROL
void QRemoteControlServer::incomingLight(QByteArray data)
{
    quint8 mode1;
    quint16 id;
    QDataStream streamIn(&data, QIODevice::ReadOnly);
    streamIn >> mode1;
    streamIn >> id;

    qDebug() << "light: id =" << id;

    if (!localSocket)
        localSocket = new QLocalSocket(this);

    if (!localSocket->isOpen() || localSocket->error() == QLocalSocket::PeerClosedError) {
        qDebug() << "Local socket closed, connecting to server..";
        localSocket->connectToServer("LightMaster",QIODevice::WriteOnly);
    }

    if (localSocket->waitForConnected(1000)) {
        qDebug() << "Local socket connected, writing data...";
        localSocket->write(data);
        localSocket->waitForBytesWritten(1000);
    }
    //delete localSocket;
}
#endif

void QRemoteControlServer::deleteConnection()
{
    if (tcpSocket == NULL)
        return;

    //tcpSocket->abort();
    tcpSocket->deleteLater();
    tcpSocket = NULL;
    ui->statusLabel->setText("Waiting for new connection...");
    setIcon("red");

#ifdef QT_DEBUG
        qDebug() << "socket disconnected";
#endif
}

void QRemoteControlServer::socketConnected()
{
    sendVersion();
    sendIcons();

    ui->statusLabel->setText("Connected");
    setIcon("green");

#ifdef QT_DEBUG
        qDebug() << "socket connected:";
#endif
}

#ifdef PLAYERCONTROL
void QRemoteControlServer::sendAmarokData()
{
    /*get metadata from amarok and use it*/
    //Status status = playerController->status().int1;

    //if (playerController->status().int1 != -1) {
#ifdef QT_DEBUG
        qDebug() << "Amarok running, reading metadata";
#endif

        //playerController->status();
        playerController->updateInfos();
        QVariantMap metadata = playerController->metadata();

        QByteArray payload;
        QDataStream dataStream(&payload, QIODevice::WriteOnly);

        dataStream << metadata.value("title").toString() << metadata.value("artist").toString();
        payload.prepend(QString("2%1").arg(payload.size(), 10, 10, QChar('0')).toLocal8Bit()); //prepending the necessary data for checking

        //qDebug() << payload.size();
        tcpSocket->write(payload);
    //}
}
#endif

void QRemoteControlServer::showEvent(QShowEvent * event)
{
    QMainWindow::showEvent(event);
    loadConfig();
}

bool QRemoteControlServer::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->shortcutDetector)
    {
        int x = 6;
        if (event->type() ==  x)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            int keyInt = keyEvent->key();
            Qt::Key key = static_cast<Qt::Key>(keyInt);
            if(key == Qt::Key_unknown){
                qDebug() << "Unknown key from a macro probably";
                return true;
            }

            // the user have clicked just and only the special keys Ctrl, Shift, Alt, Meta.
            if(key == Qt::Key_Control ||
                key == Qt::Key_Shift ||
                key == Qt::Key_Alt ||
                key == Qt::Key_Meta)
            {
                qDebug() << "Single click of special key: Ctrl, Shift, Alt or Meta";
                qDebug() << "New KeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
                return true;
            }

            // check for a combination of user clicks
            Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
            QString keyText = keyEvent->text();
            // if the keyText is empty than it's a special key like F1, F5, ...
            qDebug() << "Pressed Key:" << keyText;

            QList<Qt::Key> modifiersList;
            if(modifiers & Qt::ShiftModifier)
                keyInt += Qt::SHIFT;
            if(modifiers & Qt::ControlModifier)
                keyInt += Qt::CTRL;
            if(modifiers & Qt::AltModifier)
                keyInt += Qt::ALT;
            if(modifiers & Qt::MetaModifier)
                keyInt += Qt::META;

            currentKey = key;
            currentModifiers = modifiers;

            QString keySequence = QKeySequence(keyInt).toString(QKeySequence::NativeText);
            qDebug() << "New KeySequence:" << keySequence;
            ui->shortcutDetector->setText(keySequence);

            return true;
        }
    }
    else
    {
        int x = 6;
        if (event->type() ==  x)
        {
            ButtonSetting *targetSetting = NULL;

            for (int i = 0; i < lineEditList.size(); i++)
            {
                if (object == lineEditList.at(i))
                {
                    targetSetting = &tmpButtonSettingList[i];
                    break;
                }
            }

            if (targetSetting != NULL)
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

                int keyInt = keyEvent->key();
                Qt::Key key = static_cast<Qt::Key>(keyInt);
                if(key == Qt::Key_unknown){
                    qDebug() << "Unknown key from a macro probably";
                    return true;
                }

                // the user have clicked just and only the special keys Ctrl, Shift, Alt, Meta.
                if(key == Qt::Key_Control ||
                    key == Qt::Key_Shift ||
                    key == Qt::Key_Alt ||
                    key == Qt::Key_Meta)
                {
                    qDebug() << "Single click of special key: Ctrl, Shift, Alt or Meta";
                    qDebug() << "New KeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
                    return true;
                }

                // check for a combination of user clicks
                Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
                QString keyText = keyEvent->text();
                // if the keyText is empty than it's a special key like F1, F5, ...
                qDebug() << "Pressed Key:" << keyText;

                QList<Qt::Key> modifiersList;
                if(modifiers & Qt::ShiftModifier)
                    keyInt += Qt::SHIFT;
                if(modifiers & Qt::ControlModifier)
                    keyInt += Qt::CTRL;
                if(modifiers & Qt::AltModifier)
                    keyInt += Qt::ALT;
                if(modifiers & Qt::MetaModifier)
                    keyInt += Qt::META;

                targetSetting->key = key;
                targetSetting->modifiers = modifiers;

                QString keySequence = QKeySequence(keyInt).toString(QKeySequence::NativeText);
                qDebug() << "New KeySequence:" << keySequence;
                targetSetting->text = keySequence;

                syncButtonSettings();
                return true;
            }
        }
    }

    return QObject::eventFilter(object,event);
}

QList<ButtonSetting> QRemoteControlServer::defaultButtonSettings()
{
    QList<ButtonSetting> buttonSettings;
    ButtonSetting buttonSetting;
    // 1
    buttonSetting.key = Qt::Key_Sleep;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Sleep";
    buttonSettings.append(buttonSetting);
    // 2
    buttonSetting.key = Qt::Key_Minus;
    buttonSetting.modifiers = Qt::ControlModifier;
    buttonSetting.text = "Ctrl+-";
    buttonSettings.append(buttonSetting);
    // 3
    buttonSetting.key = Qt::Key_Plus;
    buttonSetting.modifiers = Qt::ControlModifier;
    buttonSetting.text = "Ctrl++";
    buttonSettings.append(buttonSetting);
    // 4
    buttonSetting.key = Qt::Key_VolumeMute;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Volume Mute";
    buttonSettings.append(buttonSetting);
    // 5
    buttonSetting.key = Qt::Key_VolumeDown;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Volume Down";
    buttonSettings.append(buttonSetting);
    // 6
    buttonSetting.key = Qt::Key_VolumeUp;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Volume Up";
    buttonSettings.append(buttonSetting);
    // 7
    buttonSetting.key = Qt::Key_F4;
    buttonSetting.modifiers = Qt::AltModifier;
    buttonSetting.text = "Alt+F4";
    buttonSettings.append(buttonSetting);
    // 8
    buttonSetting.key = Qt::Key_Backspace;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Backspace";
    buttonSettings.append(buttonSetting);
    // 9
    buttonSetting.key = Qt::Key_Tab;
    buttonSetting.modifiers = Qt::AltModifier;
    buttonSetting.text = "Alt+Tab";
    buttonSettings.append(buttonSetting);
    // 10
    buttonSetting.key = Qt::Key_Menu;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Menu";
    buttonSettings.append(buttonSetting);
    // 11
    buttonSetting.key = Qt::Key_Up;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Up";
    buttonSettings.append(buttonSetting);
    // 12
    buttonSetting.key = Qt::Key_Down;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Down";
    buttonSettings.append(buttonSetting);
    // 13
    buttonSetting.key = Qt::Key_Left;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Left";
    buttonSettings.append(buttonSetting);
    // 14
    buttonSetting.key = Qt::Key_Right;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Right";
    buttonSettings.append(buttonSetting);
    // 15
    buttonSetting.key = Qt::Key_Return;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Return";
    buttonSettings.append(buttonSetting);
    // 16
    buttonSetting.key = Qt::Key_MediaPrevious;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Media Previous";
    buttonSettings.append(buttonSetting);
    // 17
    buttonSetting.key = Qt::Key_MediaNext;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Media Next";
    buttonSettings.append(buttonSetting);
    // 18
    buttonSetting.key = Qt::Key_MediaPlay;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Media Play";
    buttonSettings.append(buttonSetting);
    // 19
    buttonSetting.key = Qt::Key_MediaStop;
    buttonSetting.modifiers = Qt::NoModifier;
    buttonSetting.text = "Media Stop";
    buttonSettings.append(buttonSetting);

    return buttonSettings;
}

void QRemoteControlServer::resetButtonSetting(int id)
{
    QList<ButtonSetting> defaultSettings = defaultButtonSettings();
    tmpButtonSettingList[id] = defaultSettings[id];
    syncButtonSettings();
}

void QRemoteControlServer::syncButtonSettings()
{
    for (int i = 0; i < tmpButtonSettingList.size(); i++)
        lineEditList.at(i)->setText(tmpButtonSettingList.at(i).text);
}

void QRemoteControlServer::initializeButtonConfigurator()
{
    lineEditList.append(ui->lineEdit_1);
    lineEditList.append(ui->lineEdit_2);
    lineEditList.append(ui->lineEdit_3);
    lineEditList.append(ui->lineEdit_4);
    lineEditList.append(ui->lineEdit_5);
    lineEditList.append(ui->lineEdit_6);
    lineEditList.append(ui->lineEdit_7);
    lineEditList.append(ui->lineEdit_8);
    lineEditList.append(ui->lineEdit_9);
    lineEditList.append(ui->lineEdit_10);
    lineEditList.append(ui->lineEdit_11);
    lineEditList.append(ui->lineEdit_12);
    lineEditList.append(ui->lineEdit_13);
    lineEditList.append(ui->lineEdit_14);
    lineEditList.append(ui->lineEdit_15);
    lineEditList.append(ui->lineEdit_16);
    lineEditList.append(ui->lineEdit_17);
    lineEditList.append(ui->lineEdit_18);
    lineEditList.append(ui->lineEdit_19);

    for (int i = 0; i < lineEditList.size(); i++)
        lineEditList.at(i)->installEventFilter(this);

    resetButtonList.append(ui->resetButton_1);
    resetButtonList.append(ui->resetButton_2);
    resetButtonList.append(ui->resetButton_3);
    resetButtonList.append(ui->resetButton_4);
    resetButtonList.append(ui->resetButton_5);
    resetButtonList.append(ui->resetButton_6);
    resetButtonList.append(ui->resetButton_7);
    resetButtonList.append(ui->resetButton_8);
    resetButtonList.append(ui->resetButton_9);
    resetButtonList.append(ui->resetButton_10);
    resetButtonList.append(ui->resetButton_11);
    resetButtonList.append(ui->resetButton_12);
    resetButtonList.append(ui->resetButton_13);
    resetButtonList.append(ui->resetButton_14);
    resetButtonList.append(ui->resetButton_15);
    resetButtonList.append(ui->resetButton_16);
    resetButtonList.append(ui->resetButton_17);
    resetButtonList.append(ui->resetButton_18);
    resetButtonList.append(ui->resetButton_19);

    resetButtonMapper = new QSignalMapper(this);
    for (int i = 0; i < resetButtonList.size(); i++)
    {
        resetButtonMapper->setMapping(resetButtonList[i],i);
        connect(resetButtonList[i],SIGNAL(clicked()),
                resetButtonMapper, SLOT(map()));
    }
    connect(resetButtonMapper, SIGNAL(mapped(int)),
            this, SLOT(resetButtonSetting(int)));
}

void QRemoteControlServer::saveTempChanges()
{
    int currentRow = ui->userCommandsList->currentRow();
    switch (ui->typeCombo->currentIndex()) {
        case 0: tempUserCommands[currentRow].type = CommandType;
                  tempUserCommands[currentRow].command = ui->commandEdit->text();
                  tempUserCommands[currentRow].parameters = ui->parametersEdit->text();
                  break;
        case 1: tempUserCommands[currentRow].type = ShortcutType;
                tempUserCommands[currentRow].shortcut = ui->shortcutDetector->text();
                tempUserCommands[currentRow].key = currentKey;
                tempUserCommands[currentRow].modifiers = currentModifiers;
                break;
    }
}

void QRemoteControlServer::savePassword()
{
    if (ui->passwordGroup->isChecked())
    {
        if ((!(password == ui->passwordCheckEdit->text())))
        {
            if (ui->passwordEdit->text() == ui->passwordCheckEdit->text())
            {
                //passwordHash = QCryptographicHash::hash(ui->passwordEdit->text().toUtf8(), QCryptographicHash::Sha1);
                password = ui->passwordEdit->text();
            }
            else
                QMessageBox::warning(this, tr("Error"), tr("Passwords did not match, new password not set!"));
        }
    }
    else
        password = "";
}

void QRemoteControlServer::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
         case QSystemTrayIcon::Trigger: this->show();
                break;
         case QSystemTrayIcon::DoubleClick:
             break;
         case QSystemTrayIcon::MiddleClick:
             break;
         default:
             ;
    }
}

void QRemoteControlServer::loadConfig()
{
    //load password hash
    if (!password.isEmpty())
    {
        ui->passwordEdit->setText(password);
        ui->passwordCheckEdit->setText(password);
    }
    ui->passwordGroup->setChecked(!password.isEmpty());

    //load tray icon setting
    ui->trayCheck->setChecked(trayIconVisible);

    //load autostart settings
    ui->autostartCheck->setChecked(autoStartEnabled);

    //loading the list into the listWidget
    ui->userCommandsList->clear();
    for (int i = 0; i < userCommands.size(); ++i)
    {
        QListWidgetItem *listItem = new QListWidgetItem(userCommands.at(i).name);
        if (QFile::exists(userCommands.at(i).iconPath))
            listItem->setIcon(QIcon(userCommands.at(i).iconPath));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        ui->userCommandsList->addItem(listItem);
    }

    tempUserCommands = userCommands; //Copying the list for temporary editing

    tmpButtonSettingList = buttonSettingList;
    syncButtonSettings();
}

void QRemoteControlServer::saveConfig()
{
    for (int i = 0; i < ui->userCommandsList->count(); ++i)
        tempUserCommands[i].name = ui->userCommandsList->item(i)->text();
    userCommands = tempUserCommands;

    buttonSettingList = tmpButtonSettingList;

    //save tray icon setting
    trayIconVisible = ui->trayCheck->isChecked();
    trayIcon->setVisible(trayIconVisible);

    //save autostart settings
    autoStartEnabled = ui->autostartCheck->isChecked();
    setAutostart(autoStartEnabled);

    //save modifier settings
    delayedReleaseTimer->setInterval(ui->modifierTimeoutSpin->value());

    if (tcpSocket)
        sendIcons();

    if (ui->portSpin->value() != udpSocket->localPort())
    {
        udpSocket->disconnectFromHost();
        udpSocket->bind(ui->portSpin->value(), QUdpSocket::DontShareAddress | QUdpSocket::ReuseAddressHint);
    }

    saveSettings();
}

#ifdef Q_WS_WIN
void QRemoteControlServer::checkForUpdates()
{
    QUrl url = QUrl(updateUrl);
    QString fileName = QDir::tempPath() + "/qrc_version";

    updateFile = new QFile(fileName);

    if (!updateFile->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot write update file."));
        return;
    }

    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
            this, SLOT(updateCheckFinished()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(updateCheckStarted()));
}

void QRemoteControlServer::updateCheckStarted()
{
    if (updateFile)
        updateFile->write(reply->readAll());
}

void QRemoteControlServer::updateCheckFinished()
{
    updateFile->flush();
    updateFile->close();

    if (reply->error()) {
        updateFile->remove();
        QMessageBox::information(this, tr("Error"),
                                 tr("Checking for updates failed: %1.")
                                 .arg(reply->errorString()));
    } else {
        //check version
        updateFile->open(QIODevice::ReadOnly);
        QString newestVersionString = updateFile->readLine();
        QUrl updateUrl = QUrl(updateFile->readLine());
        updateFile->close();

        newestVersionString = newestVersionString.trimmed();
        QStringList newestVersionElements = newestVersionString.split(".");

        QString currentVersionString = VERSION;
        currentVersionString = currentVersionString.trimmed();
        QStringList currentVersionElements = currentVersionString.split(".");

        bool updateNeeded = false;

        for (int i = 0; i < newestVersionElements.size(); i++)
        {
            if (newestVersionElements.at(i).toInt() > currentVersionElements.at(i).toInt())
            {
                updateNeeded = true;
                break;
            }
        }

        if (updateNeeded)
        {
            int answer = QMessageBox::information(this, tr("Update"),
                                     tr("A new version of QRemoteControl-Server is available. Would you like to download it now?"),
                                     QMessageBox::Yes | QMessageBox::No);
            if (answer == QMessageBox::Yes)
            {
                QDesktopServices::openUrl(updateUrl);
            }
        }
    }

    reply->deleteLater();
    reply = 0;
    delete updateFile;
    updateFile = 0;
}
#endif

void QRemoteControlServer::setIcon(QString name)
{
    trayIcon->setIcon(QIcon(":/trayIcons/" + name));
}

void QRemoteControlServer::setAutostart(bool enabled)
{
#ifdef Q_WS_WIN
    QSettings key("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enabled)
        key.setValue("QRC",qApp->applicationFilePath());
    else
        key.remove("QRC");
#endif
#ifdef Q_OS_LINUX
    QString autostartFileName = QDir::homePath() + "/.config/autostart/qremotecontrol.desktop";
    if (enabled)
        QFile::copy("/usr/share/applications/qremotecontrol.desktop", autostartFileName);
    else
        QFile::remove(autostartFileName);
#endif
}

void QRemoteControlServer::refreshIpAddress()
{
    QStringList hostAddresses;
    QList<QHostAddress> hosts = QNetworkInterface::allAddresses ();
      for (int i = 0; i < hosts.count(); ++i)
      {
          if ((hosts[i] != QHostAddress::LocalHost) && (hosts[i] != QHostAddress::LocalHostIPv6))
          hostAddresses.append(hosts[i].toString());
      }

      trayIcon->setToolTip(tr("IP-Addresses: %1, Port: %2").arg(hostAddresses.join(", ")).arg(udpSocket->localPort()));
}

void QRemoteControlServer::on_addUserCommand_clicked()
{
    UserCommandDialog userCommandDialog;
    userCommandDialog.setName(tr("New Command"));

    if (userCommandDialog.exec() == QDialog::Accepted)
    {
        QListWidgetItem *listItem = new QListWidgetItem(userCommandDialog.name());
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        ui->userCommandsList->addItem(listItem);

        UserCommand userCommand;
        userCommand.name = userCommandDialog.name();
        userCommand.command = userCommandDialog.command();
        userCommand.parameters = userCommandDialog.parameters();
        userCommand.type = CommandType;
        tempUserCommands.append(userCommand);

        ui->userCommandsList->setCurrentRow(ui->userCommandsList->count()-1);
    }
}

void QRemoteControlServer::on_deleteUserCommand_clicked()
{
    delete ui->userCommandsList->currentItem();
    tempUserCommands.removeAt(ui->userCommandsList->currentRow());
}

void QRemoteControlServer::on_userCommandsList_currentRowChanged(int currentRow)
{
    if (currentRow == -1) {
        ui->widget->setEnabled(false);
        return;
    }
    else
        ui->widget->setEnabled(true);

    if (QFile::exists(tempUserCommands.at(currentRow).iconPath))
        ui->iconButton->setIcon(QIcon(tempUserCommands.at(currentRow).iconPath));
    else
        ui->iconButton->setIcon(QIcon());

    switch (tempUserCommands.at(currentRow).type) {
        case CommandType: ui->typeCombo->setCurrentIndex(0);
                          ui->commandEdit->setText(tempUserCommands.at(currentRow).command);
                          ui->parametersEdit->setText(tempUserCommands.at(currentRow).parameters);
                          break;
        case ShortcutType: ui->typeCombo->setCurrentIndex(1);
                           ui->shortcutDetector->setText(tempUserCommands.at(currentRow).shortcut);
                           currentKey = tempUserCommands.at(currentRow).key;
                           currentModifiers = tempUserCommands.at(currentRow).modifiers;
                           break;
    }
}

void QRemoteControlServer::on_actionSettings_triggered()
{
    this->show();
}

void QRemoteControlServer::on_buttonBox_clicked(QAbstractButton* button)
{
    switch (ui->buttonBox->buttonRole(button)) {
        case QDialogButtonBox::AcceptRole:  savePassword();
                                            saveConfig();
                                            close();
                                            break;
        case QDialogButtonBox::ApplyRole:   savePassword();
                                            saveConfig();
                                            break;
        case QDialogButtonBox::RejectRole:  close();
                                            break;
        default:    close();
    }
}

void QRemoteControlServer::on_iconButton_clicked()
{
    IconDialog iconDialog;
    iconDialog.setSettingsDir(settingsDir);
    if (iconDialog.exec() != QDialog::Rejected)
    {
        ui->iconButton->setIcon(iconDialog.icon());
        tempUserCommands[ui->userCommandsList->currentRow()].iconPath = iconDialog.fileName();
        ui->userCommandsList->currentItem()->setIcon(iconDialog.icon());
    }
}

void QRemoteControlServer::on_browseExecutableButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName( this,
                                                     tr("Open file"),
                                                     QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                                                     tr("All Files (*)"));
    if (!fileName.isNull()) {
        ui->commandEdit->setText(fileName);
    }
}

void QRemoteControlServer::on_revertCursorButton_clicked()
{
    ui->cursorAccelerationSpin->setValue(1.5);
}

void QRemoteControlServer::on_revertScrollButton_clicked()
{
    ui->scrollSpeedSpin->setValue(0.5);
}

void QRemoteControlServer::on_revertPortButton_clicked()
{
    ui->portSpin->setValue(5487);
}

void QRemoteControlServer::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About QRemoteControl-Server"),
                       tr("<h2>QRemoteControl-Server %1</h2>"
                          "Based on Qt %3 <br><br>"
                          "Built on %2 <br><br>"
                          "Copyright 2012 by Alexander R&ouml;ssler"
                          "<br><br>"
                          "QRemoteControl-Server is free software: you can redistribute it and/or modify<br>"
                          "it under the terms of the GNU General Public License as published by<br>"
                          "the Free Software Foundation, either version 3 of the License, or<br>"
                          "(at your option) any later version.<br>"
                          "<br>"
                          "QRemoteControl-Server is distributed in the hope that it will be useful,<br>"
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of<br>"
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>"
                          "GNU General Public License for more details.<br>"
                          "<br>"
                          "You should have received a copy of the GNU General Public License<br>"
                          "along with QRemoteControl-Server.  If not, see <a href='http://www.gnu.org/licenses'>http://www.gnu.org/licenses</a>"
                          "<br><br>"
                          "For questions, improvements or bugs visit: <a href='http://qt-apps.org/content/show.php/QRemoteControl-Server?content=150421'>"
                          "http://qt-apps.org/content/show.php/QRemoteControl-Server?content=150421</a>")
                       .arg(VERSION)
                       .arg(LAST_CHANGE)
                       .arg(QT_VERSION_STR));
}

void QRemoteControlServer::on_actionHelp_triggered()
{
    HelpDialog helpDialog;
    helpDialog.exec();
}

void QRemoteControlServer::on_deleteShortcutButton_clicked()
{
    ui->shortcutDetector->clear();
    currentKey = Qt::Key_unknown;
    currentModifiers = Qt::NoModifier;
}
