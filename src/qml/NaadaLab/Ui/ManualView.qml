import QtQuick
import QtQuick.Controls

Item {
    id: root
    clip: true

    property var manualBlocks: []
    property int expectedImageCount: countImages(manualBlocks)
    property int loadedImageCount: 0
    property bool initialDelayDone: false
    property bool forceShowManual: false

    property bool minimumDelayDone: false
    property bool layoutSettled: false
    property bool initialScrollPositionApplied: false
    property real rememberedContentY: 0
    property bool restoringScrollPosition: false
    property string pendingAnchor: ""
    property var pendingAnchorIndexes: ({})

    readonly property bool manualReady:
        forceShowManual || (minimumDelayDone && layoutSettled)

    function scheduleLayoutSettling() {
        layoutSettled = false
        layoutSettleTimer.restart()
    }

    Component.onCompleted: {
        minimumDelayTimer.start()
        maxLoadingTimer.start()
        layoutSettleTimer.start()
    }

    function countImages(blocks) {
        let count = 0

        for (let i = 0; i < blocks.length; ++i) {
            if (blocks[i].type === "image")
                ++count
        }

        return count
    }

    function scrollToAnchor(anchor, anchorIndexes) {
        if (!anchorIndexes || anchorIndexes[anchor] === undefined)
            return false

        const index = Number(anchorIndexes[anchor])
        const block = blockRepeater.itemAt(index)

        if (!block || !scrollView.contentItem)
            return false

        const maximumContentY = Math.max(
            0,
            scrollView.contentItem.contentHeight
                - scrollView.contentItem.height)
        const targetContentY = Math.min(
            Math.max(0, block.y),
            maximumContentY)

        restoringScrollPosition = true
        scrollView.contentItem.cancelFlick()
        rememberedContentY = targetContentY
        scrollView.contentItem.contentY = targetContentY

        Qt.callLater(function() {
            root.restoringScrollPosition = false
        })

        return true
    }

    function highlightedLinkHtml(html, link) {
        const openingTag = "<a href=\"" + link + "\">"
        const linkStart = html.indexOf(openingTag)

        if (linkStart < 0)
            return html

        const textStart = linkStart + openingTag.length
        const linkEnd = html.indexOf("</a>", textStart)

        if (linkEnd < 0)
            return html

        return html.substring(0, textStart)
            + "<font color=\"" + Theme.link.toString() + "\">"
            + html.substring(textStart, linkEnd)
            + "</font>"
            + html.substring(linkEnd)
    }

    function activateLink(link, anchorIndexes, sourceText, sourceHtml) {
        const linkText = link.toString()

        if (linkText.startsWith("#")) {
            const anchor =
                decodeURIComponent(linkText.substring(1))

            sourceText.text = highlightedLinkHtml(sourceHtml, linkText)
            pendingAnchor = anchor
            pendingAnchorIndexes = anchorIndexes

            anchorNavigationTimer.restart()
            return
        }

        Qt.openUrlExternally(link)
    }

    onManualReadyChanged: {
        if (!manualReady)
            return

        if (!initialScrollPositionApplied) {
            if (scrollView.contentItem)
                scrollView.contentItem.contentY = 0
            rememberedContentY = 0
            initialScrollPositionApplied = true
        } else if (visible) {
            restoringScrollPosition = true
            scrollRestoreTimer.restart()
        }
    }

    onVisibleChanged: {
        if (visible && initialScrollPositionApplied) {
            restoringScrollPosition = true
            scrollRestoreTimer.restart()
        }
    }

    Timer {
        id: scrollRestoreTimer

        interval: 100
        repeat: false

        onTriggered: {
            if (!scrollView.contentItem) {
                root.restoringScrollPosition = false
                return
            }

            const maximumContentY = Math.max(
                0,
                scrollView.contentItem.contentHeight
                    - scrollView.contentItem.height)

            scrollView.contentItem.contentY = Math.min(
                root.rememberedContentY,
                maximumContentY)

            Qt.callLater(function() {
                root.restoringScrollPosition = false
            })
        }
    }

    Timer {
        id: anchorNavigationTimer

        interval: 100
        repeat: false

        onTriggered: {
            root.scrollToAnchor(
                root.pendingAnchor,
                root.pendingAnchorIndexes)
            root.pendingAnchor = ""
            root.pendingAnchorIndexes = ({})
        }
    }

    Timer {
        id: minimumDelayTimer
        interval: 300
        repeat: false

        onTriggered: {
            root.minimumDelayDone = true
        }
    }

    Timer {
        id: layoutSettleTimer
        interval: 700
        repeat: false

        onTriggered: {
            root.layoutSettled = true
        }
    }

    Timer {
        id: maxLoadingTimer
        interval: 6000
        repeat: false

        onTriggered: {
            root.forceShowManual = true
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.background

        visible: !root.manualReady
        z: 1000

        Column {
            anchors.centerIn: parent
            spacing: 12

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: !root.manualReady
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Loading..."
                color: Theme.text
                font.pixelSize: 16
            }
        }
    }

    ScrollView {
        id: scrollView

        anchors.fill: parent
        anchors.margins: 16
        clip: true

        opacity: root.manualReady ? 1.0 : 0.0

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: contentColumn

            width: Math.max(1, scrollView.availableWidth)
            spacing: 14

            onImplicitHeightChanged: {
                if (!root.initialScrollPositionApplied)
                    root.scheduleLayoutSettling()
            }

            Repeater {
                id: blockRepeater

                model: root.manualBlocks

                delegate: Item {
                    id: blockRoot

                    width: contentColumn.width

                    height: modelData.type === "image"
                            ? imageBlock.height
                            : manualText.implicitHeight

                    Text {
                        id: manualText

                        visible: modelData.type === "text"

                        width: blockRoot.width

                        text: visible ? modelData.html : ""
                        textFormat: Text.RichText
                        wrapMode: Text.WordWrap

                        color: Theme.text
                        linkColor: Theme.accent

                        font.pixelSize: 15

                        onLinkActivated: function(link) {
                            root.activateLink(
                                link,
                                modelData.anchorIndexes,
                                manualText,
                                modelData.html)
                        }
                    }

                    Item {
                        id: imageBlock

                        visible: modelData.type === "image"

                        width: blockRoot.width

                        property real naturalWidth: modelData.naturalWidth !== undefined &&
                                                    modelData.naturalWidth > 0
                                                    ? modelData.naturalWidth
                                                    : 1

                        property real naturalHeight: modelData.naturalHeight !== undefined &&
                                                     modelData.naturalHeight > 0
                                                     ? modelData.naturalHeight
                                                     : 1

                        property real displayWidth: Math.min(blockRoot.width, naturalWidth)

                        height: visible
                                ? manualImage.height
                                  + (captionText.visible ? 4 + captionText.implicitHeight : 0)
                                : 1

                        Image {
                            id: manualImage

                            visible: modelData.type === "image"

                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter

                            width: imageBlock.displayWidth
                            height: Math.round(width * imageBlock.naturalHeight / imageBlock.naturalWidth)

                            source: modelData.type === "image" ? modelData.source : ""
                            asynchronous: true

                            sourceSize.width: Math.ceil(width)
                            sourceSize.height: Math.ceil(height)

                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            cache: true
                        }

                        Text {
                            id: captionText

                            visible: modelData.captionHtml !== undefined &&
                                     modelData.captionHtml.length > 0

                            anchors.top: manualImage.bottom
                            anchors.topMargin: 4
                            anchors.horizontalCenter: manualImage.horizontalCenter

                            width: manualImage.width

                            text: visible ? modelData.captionHtml : ""
                            textFormat: Text.RichText
                            wrapMode: Text.WordWrap

                            color: Theme.secondaryText
                            font.pixelSize: 13
                            font.italic: true
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: scrollView.contentItem

        function onContentYChanged() {
            if (root.visible
                    && root.manualReady
                    && !root.restoringScrollPosition) {
                root.rememberedContentY = Math.max(
                    0,
                    scrollView.contentItem.contentY)
            }
        }
    }
}
