import QtQuick

Item {
    id: root

    property color iconColor: Theme.icon
    property string iconKind: ""
    property real iconScale: 1
    property url settingsIconSource

    TintedSvg {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset:
            root.iconKind === "configuration" ? parent.height / 4 : 0

        width: Math.round(25 * root.iconScale)
        height: width

        visible:
            root.iconKind === "settings"
            || root.iconKind === "configuration"

        source: root.settingsIconSource
        tintColor: root.iconColor
    }

    Text {
        anchors.centerIn: parent

        visible: root.iconKind === "about"
        text: "?"
        color: root.iconColor
        font.pixelSize: Math.round(23 * root.iconScale)
        font.bold: true
    }

    Canvas {
        id: iconCanvas

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset:
            root.iconKind === "configuration" ? -parent.height / 4 : 0

        width: 30
        height: 30
        scale: root.iconScale

        visible:
            root.iconKind === "midi"
            || root.iconKind === "assignment"
            || root.iconKind === "monitor"
            || root.iconKind === "configuration"
            || root.iconKind === "tracks"
            || root.iconKind === "curves"
            || root.iconKind === "manual"

        onPaint: {
            const ctx = getContext("2d")

            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = root.iconColor
            ctx.fillStyle = root.iconColor
            ctx.lineWidth = 2
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            if (root.iconKind === "midi"
                    || root.iconKind === "configuration") {
                ctx.beginPath()
                ctx.arc(15, 15, 11, 0, Math.PI * 2)
                ctx.stroke()

                const pins = [
                    [10, 11],
                    [15, 9],
                    [20, 11],
                    [11.5, 17],
                    [18.5, 17]
                ]

                for (let i = 0; i < pins.length; ++i) {
                    ctx.beginPath()
                    ctx.arc(pins[i][0], pins[i][1],
                            1.5, 0, Math.PI * 2)
                    ctx.fill()
                }
            } else if (root.iconKind === "assignment") {
                const boxes = [
                    [5, 5], [17, 5],
                    [5, 17], [17, 17]
                ]

                for (let i = 0; i < boxes.length; ++i)
                    ctx.strokeRect(boxes[i][0], boxes[i][1], 8, 8)
            } else if (root.iconKind === "monitor") {
                ctx.beginPath()
                ctx.moveTo(4, 22)
                ctx.bezierCurveTo(9, 22, 11, 9, 17, 9)
                ctx.bezierCurveTo(21, 9, 23, 17, 26, 17)
                ctx.stroke()

                ctx.beginPath()
                ctx.arc(18, 11, 2.5, 0, Math.PI * 2)
                ctx.fill()
            } else if (root.iconKind === "tracks") {
                const sliderX = [7, 15, 23]
                const sliderY = [11, 20, 15]

                for (let i = 0; i < sliderX.length; ++i) {
                    ctx.beginPath()
                    ctx.moveTo(sliderX[i], 5)
                    ctx.lineTo(sliderX[i], 25)
                    ctx.stroke()

                    ctx.fillRect(sliderX[i] - 2.5,
                                 sliderY[i] - 2,
                                 5, 4)
                }
            } else if (root.iconKind === "curves") {
                ctx.beginPath()
                ctx.moveTo(5, 8)
                ctx.bezierCurveTo(12, 8, 17, 22, 25, 22)
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(5, 22)
                ctx.bezierCurveTo(12, 22, 17, 8, 25, 8)
                ctx.stroke()
            } else if (root.iconKind === "manual") {
                ctx.beginPath()
                ctx.moveTo(4, 7)
                ctx.lineTo(13.5, 9)
                ctx.lineTo(13.5, 24)
                ctx.lineTo(4, 22)
                ctx.closePath()
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(26, 7)
                ctx.lineTo(16.5, 9)
                ctx.lineTo(16.5, 24)
                ctx.lineTo(26, 22)
                ctx.closePath()
                ctx.stroke()

                ctx.beginPath()
                ctx.moveTo(15, 9)
                ctx.lineTo(15, 24)
                ctx.stroke()
            }
        }

        Connections {
            target: root

            function onIconColorChanged() {
                iconCanvas.requestPaint()
            }

            function onIconKindChanged() {
                iconCanvas.requestPaint()
            }
        }
    }
}