import QtQuick

Item {
    id: root

    property url source
    property color tintColor: "white"

    Image {
        id: svgMask

        anchors.fill: parent
        source: root.source
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: false
        onStatusChanged: tintedImage.requestPaint()
    }

    // Paint the SVG and tint its alpha mask on the CPU. Unlike MultiEffect,
    // this also works with Qt Quick's software renderer (e.g. tablet fallback).
    Canvas {
        id: tintedImage
        anchors.fill: parent
        property url imageSource: root.source

        onImageSourceChanged: {
            if (imageSource.toString().length > 0)
                loadImage(imageSource)
            requestPaint()
        }
        onImageLoaded: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            if (!isImageLoaded(imageSource)
                    || svgMask.implicitWidth <= 0 || svgMask.implicitHeight <= 0)
                return

            const factor = Math.min(width / svgMask.implicitWidth,
                                    height / svgMask.implicitHeight)
            const w = svgMask.implicitWidth * factor
            const h = svgMask.implicitHeight * factor
            ctx.save()
            ctx.drawImage(imageSource, (width - w) / 2, (height - h) / 2, w, h)
            ctx.globalCompositeOperation = "source-in"
            ctx.fillStyle = root.tintColor
            ctx.fillRect(0, 0, width, height)
            ctx.restore()
        }

        Connections {
            target: root
            function onTintColorChanged() { tintedImage.requestPaint() }
        }
    }
}
