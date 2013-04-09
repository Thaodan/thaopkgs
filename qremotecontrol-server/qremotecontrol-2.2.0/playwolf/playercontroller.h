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

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QtCore/QVariantMap>
#include <QTimer>
#include <QDBusInterface>

struct Status{
    int int1;
    int int2;
    int int3;
    int int4;
};

class QStringList;

class PlayerController : public QObject
{
    Q_OBJECT
    public:
        PlayerController();
        ~PlayerController();
        //static PlayerController *self();

        Status status();
        void updateInfos();
        void seekForward(int seconds);
        void seekBack(int seconds);
        int volume();
        void volumeUp();
        void volumeDown();
        int rating();
        QVariantMap metadata();
        QVariantMap songMetadata(int songIndex);
        QStringList playlist(int start = 0, int numSongs = -1);
        int songPosition();
        void setAmarokPath(const QString &path);
        QString amarokPath() const;
        int position();
        void executeAmarok() const;

    public slots:
        void playPause();
        void stop();
        void prev();
        void next();
        void showAmarok();
        void seek(int pos);
        void setVolume(int volume);

    private slots:
        void updatePosition();
        void updatePlaylist(int size = 0);
        void statusChange(const Status &status);
        void trackChange(const QVariantMap &meta);

    signals:
        void metadataChanged(const QVariantMap &metadata);
        void statusChanged(int status);
        void positionChanged(int position, int time);
        void playlistChanged(const QStringList &playlist);
        void clear();

    private:
        void createInterfaces();

        QVariant playerQuery(const QString &query, const QVariant &arg = QVariant());
        QVariant playlistQuery(const QString &query);

        QDBusInterface *m_amarokTracklistDbus;
        QDBusInterface *m_amarokPlayerDbus;
        QTimer m_timer;
        QString m_customAmarokBinPath;
};

#endif
