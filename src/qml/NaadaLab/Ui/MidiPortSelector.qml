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

    LabeledComboBox {
        id: portCombo

        title: root.title
        modelData: root.ports
        currentIndex: root.indexOfPort(root.currentPort)

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