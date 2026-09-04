import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string html: ""
    property real contentMargin: 16
    property int textFontSize: 15

    signal linkActivated(string link)

    clip: true

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    Flickable {
        id: flick

        anchors.fill: parent
        anchors.margins: root.contentMargin

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: width
        contentHeight: aboutText.implicitHeight

        Text {
            id: aboutText

            width: flick.width

            text: root.html
            textFormat: Text.RichText
            wrapMode: Text.WordWrap

            color: Theme.text
            linkColor: Theme.link
            font.pixelSize: root.textFontSize

            onLinkActivated: function(link) {
                root.linkActivated(link)
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }
}