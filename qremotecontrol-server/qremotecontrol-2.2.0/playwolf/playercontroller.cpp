/*
 *   Copyright 2009 - 2010 by Giulio Camuffo <giuliocamuffo@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "playercontroller.h"

#include <QtDBus>
#include <QDBusReply>
#include <QtCore/QStringList>
/*#include <KService>
#include <KRun>
#include <KDebug>*/

Q_DECLARE_METATYPE(Status)

//QT_GLOBAL_STATIC(PlayerController, instance);

static const int UPDATE_INTERVAL = 1000;

/* DBus arguments need to handle the << and >> operators */

const QDBusArgument & operator<<(QDBusArgument &arg, const Status &change)
{
    arg.beginStructure();
    arg << change.int1 << change.int2 << change.int3 << change.int4;
    arg.endStructure();

    return arg;
}

const QDBusArgument & operator>>(const QDBusArgument &arg, Status &change)
{
    arg.beginStructure();
    arg >> change.int1 >> change.int2 >> change.int3 >> change.int4;
    arg.endStructure();

    return arg;
}

//----------------------------------------------------------------------

/*PlayerController *PlayerController::self()
{
    //return instance;
}*/

PlayerController::PlayerController()
{
    createInterfaces();
}

void PlayerController::createInterfaces()
{
    qDBusRegisterMetaType<Status>();

    m_amarokTracklistDbus = new QDBusInterface("org.kde.amarok", "/TrackList", "org.freedesktop.MediaPlayer");
    m_amarokPlayerDbus = new QDBusInterface( "org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer" );

    m_amarokPlayerDbus->connection().connect("org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer",
                                             "TrackChange", this, SLOT(trackChange(const QMap<QString, QVariant> &)) );
    m_amarokPlayerDbus->connection().connect("org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer",
                                             "StatusChange", this, SLOT(statusChange(const Status &)) );
    m_amarokTracklistDbus->connection().connect("org.kde.amarok", "/TrackList", "org.freedesktop.MediaPlayer",
                                                "TrackListChange", this, SLOT(updatePlaylist(const int&)));

    m_timer.connect(&m_timer,SIGNAL(timeout(void)), this,SLOT(updatePosition()));
    m_timer.setInterval(UPDATE_INTERVAL);
}

PlayerController::~PlayerController()
{
}

QVariant PlayerController::playerQuery(const QString &query, const QVariant &arg)
{
    if (!m_amarokPlayerDbus->isValid()) {
        createInterfaces();
    }

    QDBusReply<int> reply = m_amarokPlayerDbus->call(query, arg);
    if (reply.isValid()) {
        return reply.value();
    }

    return QVariant();
}

QVariant PlayerController::playlistQuery(const QString &query)
{
    QDBusReply<int> reply = m_amarokTracklistDbus->call( query );
    if (reply.isValid()) {
        return reply.value();
    }

    return -1;
}

Status PlayerController::status()
{
    qDBusRegisterMetaType<Status>();

    if (!m_amarokPlayerDbus->isValid()) {
        createInterfaces();
    }

    QDBusReply<Status> reply = m_amarokPlayerDbus->call("GetStatus");
    if (reply.isValid()) {
        return reply.value();
    }

    Status st;
    st.int1 = -1;
    st.int2 = -1;
    st.int3 = -1;
    st.int4 = -1;
    return st;
}

int PlayerController::songPosition()
{
    return playlistQuery("GetCurrentTrack").toInt();
}

QVariantMap PlayerController::songMetadata(int songIndex)
{
    QDBusReply<QMap<QString, QVariant> > argumentList = m_amarokTracklistDbus->call("GetMetadata", songIndex);
    if (argumentList.isValid()) {
        return (argumentList.value());
    }

    return QVariantMap();
}

QVariantMap PlayerController::metadata()
{
    return songMetadata(songPosition());
}

QStringList PlayerController::playlist(int start, int numSongs)
{
    QStringList titleList;
    int startIndex = start;
    int i = 0;
    do {
        const QVariantMap meta = songMetadata(startIndex);
        if (meta != QVariantMap()) {
            titleList << meta.value("title").toString();
        } else {
            break;
        }

        ++startIndex;
        ++i;

        if ((i == numSongs) and (numSongs != -1)) {
            break;
        }
    } while (true);

    return titleList;
}

void PlayerController::updatePlaylist(const int size)
{
    Q_UNUSED(size);

    emit playlistChanged(playlist(songPosition() + 1 , size));
}

void PlayerController::playPause()
{
    if (status().int1 == -1) {
        executeAmarok();

        if(status().int1 != 0) {
            playerQuery("Play");
        }
    }

    if (status().int1 == 0) {
        playerQuery("Pause");
    } else {
        playerQuery("Play");
    }

}

void PlayerController::prev()
{
    playerQuery("Prev");
}

void PlayerController::next()
{
    playerQuery("Next");
}

void PlayerController::stop()
{
    playerQuery("Stop");
}

void PlayerController::showAmarok()
{
    QDBusInterface amarok( "org.kde.amarok", "/amarok/MainWindow", "org.kde.amarok.MainWindow" );

    if (status().int1 == -1) {
        executeAmarok();
    } else {
        amarok.call("showHide");
    }
}

void PlayerController::executeAmarok() const
{
    /*const KService::Ptr amarokService = KService::serviceByStorageId("amarok");

    if (!m_customAmarokBinPath.isNull()) {
        if (amarokService) {
            new KRun(KUrl(amarokService->entryPath()), 0);
        }
    } else {
        new KRun(KUrl(m_customAmarokBinPath), 0);
    }*/
}

void PlayerController::seek(int pos)
{
    playerQuery("PositionSet", pos);
}

void PlayerController::seekForward(int seconds)
{
    seek(playerQuery("PositionGet").toInt() + seconds * 1000);
}

void PlayerController::seekBack(int seconds)
{
    int time = playerQuery("PositionGet").toInt() - seconds * 1000;

    if (time >= 0) {
        seek(time);
    } else {
        seek(1);
    }
}

void PlayerController::setVolume(int volume)
{
    playerQuery("VolumeSet", volume);
}

int PlayerController::volume()
{
    return playerQuery("VolumeGet").toInt();
}

void PlayerController::volumeUp()
{
    setVolume(volume() + 5);
}

void PlayerController::volumeDown()
{
    setVolume(volume() - 5);
}

int PlayerController::rating()
{
    return metadata().value("rating").toInt();
}

void PlayerController::updateInfos()
{
    statusChange(status());
}

void PlayerController::trackChange(const QVariantMap &meta)
{
    if (status().int1 == 2) {
        return;
    }

    emit metadataChanged(meta);

    updatePosition();
    updatePlaylist();
}

void PlayerController::statusChange(const Status &status)
{
    int stat = status.int1;
    emit statusChanged(stat);

    if (stat == 0) {
        m_timer.start();
        trackChange(metadata());
    } else if (stat == 1) {
        m_timer.stop();
        trackChange(metadata());
    } else {
        m_timer.stop();
        emit clear();
    }
}

int PlayerController::position()
{
    return playerQuery("PositionGet").toInt();
}

void PlayerController::updatePosition()
{
    emit positionChanged(position(), metadata().value("mtime").toInt());
}

void PlayerController::setAmarokPath(const QString &path)
{
    m_customAmarokBinPath = path;
}

QString PlayerController::amarokPath() const
{
    return m_customAmarokBinPath;
}

//#include "playercontroller.moc"
