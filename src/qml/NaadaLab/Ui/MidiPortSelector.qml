import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    property string title: "MIDI Port"
    property var ports: []
    property string currentPort: ""
    property string refreshText: "Refresh"

    signal portSelected(string portName)
    signal refreshRequested()

    spacing: 8

    function indexOfPort(portName) {
        for (let i = 0; i < ports.length; ++i) {
            if (ports[i] === portName)
                return i
        }

        return -1
    }

    // ComboBox may choose index 0 while applying a new model, after a binding
    // to currentIndex has already evaluated. Reconcile by name once that model
    // update has settled. Only user activation emits portSelected.
    function syncSelection() {
        portCombo.currentIndex = root.indexOfPort(root.currentPort)
    }

    onPortsChanged: Qt.callLater(root.syncSelection)
    onCurrentPortChanged: Qt.callLater(root.syncSelection)
    Component.onCompleted: Qt.callLater(root.syncSelection)

    LabeledComboBox {
        id: portCombo

        title: root.title
        modelData: root.ports
        currentIndex: -1

        Layout.fillWidth: true
        Layout.preferredWidth: 2

        onActivated: function(index) {
            if (index >= 0 && index < root.ports.length)
                root.portSelected(root.ports[index])
        }
    }

    ActionButton {
        text: root.refreshText

        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.preferredHeight: Theme.controlHeight
        Layout.alignment: Qt.AlignBottom

        onClicked: root.refreshRequested()
    }
}