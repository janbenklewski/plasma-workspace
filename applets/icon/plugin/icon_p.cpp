/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "icon_p.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDropEvent>
#include <QFileInfo>
#include <QJsonArray>
#include <QMimeData>

#include <KFileItem>
#include <KDesktopFile>
#include <KRun>
#include <KService>
#include <KShell>
#include <QMimeType>
#include <QMimeDatabase>

#include <KIO/Global>
#include <KIO/DropJob>
#include <KJobWidgets>

IconPrivate::IconPrivate() {
}

IconPrivate::~IconPrivate() {
}

void IconPrivate::setUrl(const QUrl &url)
{
    m_url = url;

    if (m_url.isLocalFile()) {
        const KFileItem fileItem(m_url);
        const QFileInfo fi(m_url.toLocalFile());

        if (fileItem.isDesktopFile()) {
            const KDesktopFile f(m_url.toLocalFile());
            m_name = f.readName();
            m_icon = f.readIcon();
            m_genericName = f.readGenericName();
            if (m_name.isNull()) {
                m_name = QFileInfo(m_url.toLocalFile()).fileName();
            }
        } else {
            QMimeDatabase db;
            m_name = fi.baseName();
            m_icon = db.mimeTypeForUrl(m_url).iconName();
            m_genericName = fi.baseName();
        }
    } else {
        if (m_url.scheme().contains("http")) {
            m_name = m_url.host();
        } else if (m_name.isEmpty()) {
            m_name = m_url.toString();
            if (m_name.endsWith(QLatin1String(":/"))) {
                m_name = m_url.scheme();
            }
        }
        m_icon = KIO::iconNameForUrl(url);
    }

    emit urlChanged(m_url);
    emit nameChanged(m_name);
    emit iconChanged(m_icon);
    emit genericNameChanged(m_genericName);
}

QUrl IconPrivate::url() const
{
    return m_url;
}

QString IconPrivate::name() const
{
    return m_name;
}

QString IconPrivate::icon() const
{
    return m_icon;
}

QString IconPrivate::genericName() const
{
    return m_genericName;
}

bool IconPrivate::processDrop(QObject *dropEvent)
{
    Q_ASSERT(dropEvent);

    if (!m_url.isLocalFile()) {
        return false;
    }

    // DeclarativeDropEvent and co aren't public
    const QObject *mimeData = qvariant_cast<QObject *>(dropEvent->property("mimeData"));
    Q_ASSERT(mimeData);

    const QJsonArray &droppedUrls = mimeData->property("urls").toJsonArray();

    QList<QUrl> urls;
    urls.reserve(droppedUrls.count());
    foreach (const QJsonValue &droppedUrl, droppedUrls) {
        const QUrl url = QUrl::fromUserInput(droppedUrl.toString(), QString(), QUrl::AssumeLocalFile);
        if (url.isValid()) {
            urls.append(url);
        }
    }

    if (urls.isEmpty()) {
        return false;
    }

    const QString stringUrl = m_url.toLocalFile();

    QMimeDatabase db;
    const QMimeType mimeType = db.mimeTypeForFile(stringUrl);

    if (KDesktopFile::isDesktopFile(stringUrl)) {
        const KDesktopFile desktopFile(stringUrl);
        const QStringList &supportedMimeTypes = desktopFile.readMimeTypes();

        // if no mime types are given just execute the command in the Desktop file
        if (supportedMimeTypes.isEmpty()) {
            KService service(stringUrl);
            KRun::runService(service, urls, nullptr);
            return true;
        }

        // otherwise check if the applicaton supports the dropped type
        foreach (const QString &supportedType, supportedMimeTypes) {
            if (mimeType.inherits(supportedType)) {
                KService service(stringUrl);
                KRun::runService(service, urls, nullptr);
                return true;
            }
        }

        return false;
    }

    if (mimeType.inherits("application/x-executable") || mimeType.inherits("application/x-shellscript")) {
        QString params;
        foreach (const QUrl &url, urls) {
            // TODO toEncoded?
            params += QLatin1Char(' ') + KShell::quoteArg(url.isLocalFile() ? url.toLocalFile() : url.toEncoded());
        }

        KRun::runCommand(KShell::quoteArg(m_url.path()) + QLatin1Char(' ') + params, nullptr);
        return true;
    } else if (mimeType.inherits("inode/directory")) {
        QMimeData mimeData;
        mimeData.setUrls(urls);

        // DeclarativeDropEvent isn't public
        QDropEvent de(QPointF(dropEvent->property("x").toInt(), dropEvent->property("y").toInt()),
                      static_cast<Qt::DropActions>(dropEvent->property("proposedActions").toInt()),
                      &mimeData,
                      static_cast<Qt::MouseButtons>(dropEvent->property("buttons").toInt()),
                      static_cast<Qt::KeyboardModifiers>(dropEvent->property("modifiers").toInt()));

        KIO::DropJob *dropJob = KIO::drop(&de, m_url);
        KJobWidgets::setWindow(dropJob, QApplication::desktop());
    }

    return false;
}

void IconPrivate::open()
{
    new KRun(m_url, 0);
}

