import QtQuick
import QtQuick.Controls

Item {
    id: root

    property bool selected: false
    property bool keepLoaded: true
    property string pageTitle: ""
    property Component pageComponent
    property int revealDelayMs: 24
    property int activationDelayMs: 0
    property bool asynchronousLoading: true

    property bool loadedOnce: false
    property bool revealContent: false
    property bool activationRequested: false

    onSelectedChanged: {
        if (selected) {
            revealContent = false

            if (loadedOnce && keepLoaded) {
                activationRequested = true
            } else if (activationDelayMs > 0) {
                activationRequested = false
                activationTimer.restart()
            } else {
                activationRequested = true
            }

            revealTimer.restart()
        } else {
            activationTimer.stop()
            revealTimer.stop()
            revealContent = false

            if (!keepLoaded)
                activationRequested = false
        }
    }

    Timer {
        id: activationTimer
        interval: root.activationDelayMs
        repeat: false
        onTriggered: root.activationRequested = true
    }

    Timer {
        id: revealTimer
        interval: root.revealDelayMs
        repeat: false
        onTriggered: root.revealContent = true
    }

    Loader {
        id: pageLoader

        anchors.fill: parent
        active: root.activationRequested
                || (root.keepLoaded && root.loadedOnce)
        asynchronous: root.asynchronousLoading
        sourceComponent: root.pageComponent
        visible: root.selected
                 && root.revealContent
                 && status === Loader.Ready

        onLoaded: root.loadedOnce = true
    }

    Rectangle {
        anchors.fill: parent
        visible: root.selected
                 && (!root.revealContent
                     || pageLoader.status !== Loader.Ready)
        color: Theme.background
        z: 2

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 16

            text: root.pageTitle
            color: Theme.secondaryText
            font.pixelSize: 15
            font.bold: true
        }

        BusyIndicator {
            anchors.centerIn: parent
            visible: pageLoader.status === Loader.Loading
            running: visible
        }

        Text {
            anchors.centerIn: parent
            visible: pageLoader.status === Loader.Error
            text: qsTr("Unable to load this page")
            color: Theme.text
            font.pixelSize: 14
        }
    }
}