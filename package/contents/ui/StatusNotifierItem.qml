/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.ToolTipArea {
    id: taskIcon
    height: hidden ? root.hiddenItemSize : root.itemSize
    width: labelVisible ? parent.width : height
    property bool labelVisible: taskIcon.hidden && !root.activeApplet

    mainText: ToolTipTitle
    subText: ToolTipSubTitle
    icon: ToolTipIcon ? ToolTipIcon : plasmoid.nativeInterface.resolveIcon(IconName != "" ? IconName : Icon, IconThemePath)

    location: if (taskIcon.parent && taskIcon.parent.objectName == "hiddenTasksColumn") {
                return PlasmaCore.Types.RightEdge;
              } else {
                return taskIcon.location;
              }

    property bool hidden: parent.objectName == "hiddenTasksColumn"

    PlasmaCore.IconItem {
        id: iconItem
        source: plasmoid.nativeInterface.resolveIcon(IconName != "" ? IconName : Icon, IconThemePath)
        width: Math.min(parent.width, parent.height)
        height: width

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
    }

    MouseArea {
        anchors.fill: taskIcon
        hoverEnabled: true
        onClicked: {
            //print(iconSvg.hasElement(IconName))
            var service = statusNotifierSource.serviceForSource(DataEngineSource)
            var operation = service.operationDescription("Activate")
            operation.x = parent.x

            // kmix shows main window instead of volume popup if (parent.x, parent.y) == (0, 0), which is the case here.
            // I am passing a position right below the panel (assuming panel is at screen's top).
            // Plasmoids' popups are already shown below the panel, so this make kmix's popup more consistent
            // to them.
            operation.y = parent.y + parent.height + 6
            service.startOperationCall(operation)
        }
        onEntered: {
            if (hidden) {
                root.hiddenLayout.hoveredItem = taskIcon
            }
        }
    }
    PlasmaComponents.Label {
        opacity: labelVisible ? 1 : 0
        x: iconItem.width + units.smallSpacing
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            verticalCenter: parent.verticalCenter
        }
        text: Title
    }
}
