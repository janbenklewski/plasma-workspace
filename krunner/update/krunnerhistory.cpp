/***************************************************************************
 *   Copyright (C) 2020 by Alexander Lohnau <alexander.lohnau@gmx.de>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QCoreApplication>
#include <QTimer>

#include <KActivities/Consumer>
#include <KSharedConfig>
#include <KConfigGroup>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    auto consumer = new KActivities::Consumer();
    // Wait a bit for consumer to be initialized
    QTimer::singleShot(5, [consumer](){
        const QString history = KSharedConfig::openConfig("krunnerrc")->group("General").readEntry("history");
        const QStringList activities = consumer->activities();
        KConfigGroup newHistory = KSharedConfig::openConfig("krunnerrc")->group("PlasmaRunnerManager").group("History");
        for (const QString &activity : activities) {
            newHistory.writeEntry(activity, history);
        }
        newHistory.sync();
        qApp->exit();
    });

    return QCoreApplication::exec();
}
