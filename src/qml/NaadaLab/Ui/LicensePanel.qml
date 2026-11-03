import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var license
    required property string appName
    property bool expanded: true
    property bool showBadge: true
    property bool referenceCopied: false
    property bool reviewExpanded: false
    readonly property bool reviewing: info.state === "review"
    visible: license.enabled
    readonly property var info: license.status
    readonly property bool paid: info.state === "purchased"
    readonly property bool pending: !!info.purchasePending
    readonly property string heading: pending ? qsTr("Payment pending")
        : reviewing ? qsTr("Review access unlocked")
        : paid ? qsTr("Full version unlocked")
        : info.state === "trial" ? qsTr("Your trial is active")
        : info.state === "expired" ? qsTr("Your trial has ended")
        : info.state === "checking" ? qsTr("Checking access")
        : info.state === "not_configured" ? qsTr("Purchases unavailable")
        : qsTr("Try %1 free for 7 days").arg(appName)
    component ActionButton: Button {
        id: control
        property bool primary: false
        leftPadding: 16; rightPadding: 16
        background: Rectangle {
            radius: 6
            color: !control.enabled ? "#202b36" : control.down ? "#9a803f" : control.primary ? "#e7c466" : "#203246"
            border.color: control.enabled ? "#ad965a" : "#43505e"
        }
        contentItem: Text {
            text: control.text; font: control.font
            color: !control.enabled ? "#87929e" : control.primary ? "#101820" : "#f4f5f6"
            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
    }
    Connections {
        target: root.license
        function onAccessGranted() { root.expanded = false }
    }
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        width: badge.implicitWidth + 24
        height: 44
        radius: 6
        color: "#182b40"
        border.color: "#d8b85a"
        visible: root.showBadge && !root.expanded
        Text {
            id: badge
            anchors.centerIn: parent
            text: root.reviewing ? qsTr("Review access") : root.paid ? qsTr("Full version") : root.pending ? qsTr("Payment pending")
                : root.license.allowed ? qsTr("Trial · View options") : qsTr("Unlock MIDI")
            color: "#f4d477"
        }
        MouseArea { anchors.fill: parent; onClicked: root.expanded = true }
    }
    Rectangle {
        anchors.fill: parent
        color: "#fa101820"
        visible: root.expanded
        MouseArea { anchors.fill: parent }
        ScrollView {
            id: scroll
            anchors.fill: parent
            anchors.margins: Math.min(24, root.width * 0.05)
            contentWidth: availableWidth
            clip: true
            ColumnLayout {
                width: Math.min(640, scroll.availableWidth)
                x: Math.max(0, (scroll.availableWidth - width) / 2)
                spacing: 12
                Label { text: root.appName; font.pixelSize: 26; color: "#f1ce6d"; Layout.fillWidth: true }
                Label {
                    text: root.heading
                    font.pixelSize: 22; font.bold: true
                    color: "white"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.info.state === "not_started" && !root.pending
                    text: qsTr("All MIDI features. No subscription or automatic charge.")
                    font.pixelSize: 16; color: "#eeeeee"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: !!root.info.message
                    text: root.info.message || ""
                    font.pixelSize: 16; color: "#eeeeee"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.info.state === "trial" && root.info.trialEndsAt > 0
                    text: qsTr("Trial ends: %1").arg(new Date(root.info.trialEndsAt * 1000).toLocaleString())
                    font.pixelSize: 16; color: "#f1ce6d"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.info.state === "expired"
                    text: root.license.allowed
                        ? qsTr("You can finish this session. Unlock the full version to use MIDI next time. Your presets are kept.")
                        : qsTr("Unlock the full version to use MIDI again. Your presets are kept, and you can still explore the app.")
                    font.pixelSize: 16; color: "#eeeeee"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 10
                    ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                        primary: true
                        text: qsTr("Start free trial")
                        visible: root.info.state === "not_started" && !root.pending
                        enabled: !!root.info.configured && !root.info.busy
                        onClicked: root.license.activate()
                    }
                    ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                        primary: root.info.state !== "not_started"
                        text: root.info.price ? qsTr("Buy · %1").arg(root.info.price) : qsTr("Buy full version")
                        visible: !root.paid && !root.reviewing && !root.pending
                        enabled: !!root.info.canBuy && !root.info.busy
                        onClicked: root.license.buy()
                    }
                    ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                        text: root.pending ? qsTr("Check payment") : qsTr("Restore / check access")
                        enabled: !!root.info.configured && !root.info.busy
                        onClicked: root.license.restore()
                    }
                }
                Label {
                    visible: !root.paid && !root.reviewing && !root.pending && !!root.info.productMessage
                    text: root.info.productMessage || ""
                    font.pixelSize: 15; color: "#e7c466"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.info.state === "expired" && root.info.trialAvailableAt > 0
                    text: qsTr("Another free trial is available from %1. Use Restore / check access while online, then choose Start free trial.").arg(new Date(root.info.trialAvailableAt * 1000).toLocaleString())
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: !root.paid && !root.reviewing && !root.pending
                    text: qsTr("One-time purchase for %1. No subscription.").arg(root.appName)
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.pending
                    text: qsTr("No need to buy again. Full access starts when Google Play confirms the payment.")
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                    text: root.license.allowed ? qsTr("Continue to %1").arg(root.appName) : qsTr("Explore without MIDI")
                    onClicked: root.expanded = false
                }
                Label {
                    text: root.reviewing ? qsTr("All features are available offline. After reinstalling, enter the same review code again.") : root.paid
                        ? qsTr("You can use this app offline. To restore it on another device, install it with the same Google Play account and choose Restore / check access while online.")
                        : qsTr("Go online to start the trial, buy or restore a purchase. During the trial, reconnect after restarting your device. After purchase verification, you can use the app offline.")
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Label {
                    visible: root.info.state === "not_started" && !root.pending
                    text: qsTr("The 7 days start only when you choose Start free trial. NaadaLab records an app-specific device identifier to manage the trial; Google Play verifies the installation.")
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 10
                    ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                        text: qsTr("Privacy")
                        enabled: !!root.info.privacyUrl
                        onClicked: Qt.openUrlExternally(root.info.privacyUrl)
                    }
                    ActionButton {
                        font.pixelSize: 14; implicitHeight: 48
                        text: qsTr("Support")
                        onClicked: Qt.openUrlExternally("mailto:feedback@naadalab.com?subject=" + encodeURIComponent(root.appName + " support") + "&body=" + encodeURIComponent("Support reference: " + (root.info.supportReference || "Unavailable") + "\n\n"))
                    }
                }
                Label {
                    visible: !!root.info.supportReference
                    text: qsTr("For help or a privacy request, copy your support reference and send it to feedback@naadalab.com. It identifies this app on this device; it is not a password or proof of ownership.")
                    font.pixelSize: 15; color: "#c5cdd7"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                }
                ActionButton {
                    visible: !!root.info.supportReference
                    text: root.referenceCopied ? qsTr("Reference copied") : qsTr("Copy support reference")
                    font.pixelSize: 14; implicitHeight: 48
                    onClicked: { root.referenceCopied = root.license.copySupportReference(); copiedReset.restart() }
                }
                Timer { id: copiedReset; interval: 4000; onTriggered: root.referenceCopied = false }
                GridLayout {
                    id: reviewRow
                    visible: !root.reviewing
                    Layout.fillWidth: true
                    columns: width >= 480 ? 2 : 1
                    columnSpacing: 12
                    rowSpacing: 12
                    ActionButton {
                        text: root.reviewExpanded ? qsTr("Hide review access") : qsTr("Review access")
                        font.pixelSize: 14; implicitHeight: 48
                        onClicked: {
                            root.reviewExpanded = !root.reviewExpanded
                            if (root.reviewExpanded)
                                Qt.callLater(function() { revealReview.restart() })
                        }
                    }
                    TextField {
                        id: reviewCode
                        visible: root.reviewExpanded
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        implicitHeight: 48
                        font.pixelSize: 16
                        color: "#ffffff"
                        placeholderTextColor: "#aeb9c6"
                        leftPadding: 12; rightPadding: 12
                        background: Rectangle { color: "#152334"; radius: 6; border.color: reviewCode.activeFocus ? "#e7c466" : "#738397" }
                        placeholderText: qsTr("Paste review code")
                        maximumLength: 1024
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                        selectByMouse: true
                        onAccepted: { root.license.unlockReview(text); clear() }
                    }
                }
                ColumnLayout {
                    visible: root.reviewExpanded && !root.reviewing
                    Layout.fillWidth: true
                    Label {
                        text: qsTr("For app reviewers: paste the access code provided in Play Console.")
                        color: "#eeeeee"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    ActionButton {
                        text: qsTr("Unlock review access")
                        enabled: !!root.info.configured && reviewCode.text.trim().length > 0
                        font.pixelSize: 14; implicitHeight: 48
                        onClicked: { root.license.unlockReview(reviewCode.text); reviewCode.clear() }
                    }
                }
                Timer {
                    id: revealReview
                    interval: 50
                    onTriggered: {
                        if (!root.reviewExpanded) return
                        var flick = scroll.contentItem
                        var fieldBottom = reviewCode.mapToItem(flick.contentItem, 0, reviewCode.height).y
                        var limit = Math.max(0, flick.contentHeight - flick.height)
                        flick.contentY = Math.max(0, Math.min(limit, Math.max(flick.contentY, fieldBottom + 12 - flick.height)))
                    }
                }
                BusyIndicator { running: root.info.busy; visible: running; Layout.preferredHeight: 36 }
            }
        }
    }
}
