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
#ifndef QREMOTECONTROLSERVER_H
#define QREMOTECONTROLSERVER_H

#define LAST_CHANGE __DATE__ " " __TIME__

//#define PORTABLE

#include <QMainWindow>
#include <QtNetwork>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QAbstractButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QLocalSocket>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>
#include <QMenu>
#include "usercommanddialog.h"
#include "helpdialog.h"
#include "icondialog.h"

//#ifdef Q_OS_LINUX
//#define PLAYERCONTROL
//#endif

#ifdef PLAYERCONTROL
#include "./playwolf/playercontroller.h"
#endif

enum UserCommandType { CommandType, ShortcutType};

struct UserCommand {
    UserCommandType type;
    QString name,
            iconPath,
            command,
            parameters,
            shortcut;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers;
};

typedef struct {
    QString text;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers;
} ButtonSetting;

namespace Ui {
    class QRemoteControlServer;
}

class QRemoteControlServer : public QMainWindow {
    Q_OBJECT
public:
    QRemoteControlServer(QWidget *parent = 0);
    ~QRemoteControlServer();

protected:
    void changeEvent(QEvent *e);
    void showEvent (QShowEvent * event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::QRemoteControlServer    *ui;
    QTcpSocket                  *tcpSocket;
    QUdpSocket                  *udpSocket;
    QSystemTrayIcon             *trayIcon;
    bool                        trayIconVisible;
    QList<UserCommand>          userCommands;
    QList<UserCommand>          tempUserCommands;

    //QByteArray passwordHash;
    QString password;
    QString settingsDir;

    Qt::Key                 currentKey;
    Qt::KeyboardModifiers   currentModifiers;

    QTimer               *delayedReleaseTimer;
    Qt::KeyboardModifiers delayedReleaseModifiers;

    //button configuration
    QList<ButtonSetting>    buttonSettingList;
    QList<ButtonSetting>    tmpButtonSettingList;
    QList<QLineEdit*>       lineEditList;
    QList<QToolButton*>     resetButtonList;
    QSignalMapper           *resetButtonMapper;

    QPointF cursorPos;
    bool    autoStartEnabled;

    //network
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QFile *updateFile;

    void initializeTrayIcon();

#ifdef PLAYERCONTROL
    PlayerController *playerController;
#endif

#ifdef LIGHTCONTROL
    QLocalSocket *localSocket;
void incomingLight(QByteArray data);
#endif

    //button functions
    QList<ButtonSetting> defaultButtonSettings();
    void initializeButtonConfigurator();
    void syncButtonSettings();
    void storeButtonSettings();

    //some functionss
    void setIcon(QString name);
    void setAutostart(bool enabled);

    //incoming network functions
    void incomingKey(QByteArray data);
    void incomingMouse(QByteArray data);
    void incomingAction(QByteArray data);
    void incomingButton(QByteArray data);

    void incomingConnectionRequest();
    void incomingData();

    //outgoing network functionss
    void sendIcons();
    void sendVersion();
#ifdef PLAYERCONTROL
    void sendAmarokData();
#endif

    void sendKeyAndModifiers(Qt::Key key, Qt::KeyboardModifiers modifiers, bool keyPressed);

    //setting loading and saving to ini
    void loadSettings();
    void saveSettings();

    //loading and saving settings to GUI
    void savePassword();
    void loadConfig();
    void saveConfig();

private slots:
    void incomingUdpData();
    void incomingButtonDelayedRelease();
    void deleteConnection();
    void socketConnected();
    void saveTempChanges();
    void refreshIpAddress();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    //button configuration
    void resetButtonSetting(int id);

    //single application
    void applicationStarted(const QString &message);

    //update checking, only enabled in Windows
#ifdef Q_WS_WIN
    void checkForUpdates();
    void updateCheckStarted();
    void updateCheckFinished();
#endif

    void on_browseExecutableButton_clicked();
    void on_deleteUserCommand_clicked();
    void on_iconButton_clicked();
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_userCommandsList_currentRowChanged(int currentRow);
    void on_addUserCommand_clicked();
    void on_actionSettings_triggered();
    void on_revertCursorButton_clicked();
    void on_revertScrollButton_clicked();
    void on_revertPortButton_clicked();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_deleteShortcutButton_clicked();
};

#endif // QREMOTECONTROLSERVER_H
