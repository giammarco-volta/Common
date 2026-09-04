import QtQuick
import QtQuick.Effects

Item {
    id: root

    property url source
    property color tintColor: "white"

    Rectangle {
        id: tintSource

        anchors.fill: parent
        color: root.tintColor
        visible: false
    }

    Image {
        id: svgMask

        anchors.fill: parent
        source: root.source
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: false
    }

    MultiEffect {
        anchors.fill: parent
        source: tintSource
        maskEnabled: true
        maskSource: svgMask
    }
}