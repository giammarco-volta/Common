import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var sections: []
    property string currentSection: ""
    property real railWidth: 56
    property real buttonHeight: 56
    property real iconScale: Math.min(1, buttonHeight / 56)
    property url settingsIconSource

    signal sectionActivated(string section)
    signal sectionPressAndHold(string section)

    implicitWidth: railWidth
    color: Qt.darker(Theme.background, 1.15)

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        width: 1
        color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: root.sections

            delegate: ToolButton {
                id: sectionButton

                required property int index
                required property var modelData

                readonly property bool selected:
                    root.currentSection === modelData.section

                readonly property real heightUnits:
                    modelData.heightUnits || 1

                Layout.fillWidth: true
                Layout.preferredHeight:
                    root.buttonHeight * heightUnits
                Layout.minimumHeight:
                    root.buttonHeight * heightUnits
                Layout.maximumHeight:
                    root.buttonHeight * heightUnits

                hoverEnabled: true

                onPressed:
                    root.sectionActivated(modelData.section)

                onPressAndHold:
                    root.sectionPressAndHold(modelData.section)

                ToolTip.visible: hovered
                ToolTip.text: modelData.name
                ToolTip.delay: 400

                background: Rectangle {
                    color:
                        sectionButton.selected
                            ? Theme.selection
                            : "transparent"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        width: 3
                        color:
                            sectionButton.selected
                                ? Theme.accent
                                : "transparent"
                    }
                }

                contentItem: NavigationIcon {
                    iconColor:
                        sectionButton.selected
                            ? Theme.accent
                            : Theme.icon

                    iconKind:
                        sectionButton.modelData.icon
                            || sectionButton.modelData.section

                    iconScale: root.iconScale
                    settingsIconSource: root.settingsIconSource
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}