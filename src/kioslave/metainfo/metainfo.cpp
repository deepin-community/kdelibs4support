/*  This file is part of the KDE libraries
    Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "metainfo.h"

#include <QCoreApplication>
#include <QUrl>
#include <QDebug>
#include <QDataStream>
#include <kfilemetainfo.h>
#include <klocalizedstring.h>
#include <stdlib.h>

// Recognized metadata entries:
// mimeType     - the mime type of the file, so we need not extra determine it
// what         - what to load

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.metainfo" FILE "metainfo.json")
};

using namespace KIO;

extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    QCoreApplication::setApplicationName("kio_metainfo");

    QCoreApplication app(argc, argv);

    //KApplication app(argc, argv, "kio_metainfo", false, true);

    if (argc != 4) {
        qCritical() << "Usage: kio_metainfo protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    MetaInfoProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    return 0;
}

MetaInfoProtocol::MetaInfoProtocol(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("metainfo", pool, app)
{
}

MetaInfoProtocol::~MetaInfoProtocol()
{
}

void MetaInfoProtocol::get(const QUrl &url)
{
    QString mimeType = metaData("mimeType");
    KFileMetaInfo info(url.toLocalFile(), mimeType);

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);

    stream << info;

    data(arr);
    finished();
}

void MetaInfoProtocol::put(const QUrl &url, int, KIO::JobFlags)
{
    QString mimeType = metaData("mimeType");
    KFileMetaInfo info;

    QByteArray arr;
    readData(arr);
    QDataStream stream(arr);

    stream >> info;

    if (info.isValid()) {
        info.applyChanges();
    } else {
        error(ERR_NO_CONTENT, i18n("No metainfo for %1", url.path()));
        return;
    }
    finished();
}

#include "metainfo.moc"