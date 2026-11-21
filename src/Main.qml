import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 520
    height: 560
    minimumWidth: 380
    minimumHeight: 440
    visible: true
    title: "Pomo"
    color: "#0e0e10"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    property bool helpVisible: false

    // Reading translator.language makes these re-render on a language switch.
    function phaseName(p) {
        translator.language
        return p === 0 ? qsTr("Focus") : p === 1 ? qsTr("Short break") : qsTr("Long break")
    }
    function clock(s) {
        translator.language
        const m = Math.floor(s / 60), r = s % 60
        return Qt.locale().toString(m).padStart(2, "0") + ":" + Qt.locale().toString(r).padStart(2, "0")
    }

    Shortcut { sequence: "?"; context: Qt.ApplicationShortcut; onActivated: win.helpVisible = !win.helpVisible }
    Shortcut { sequence: "Escape"; context: Qt.ApplicationShortcut; onActivated: win.helpVisible = false }
    Shortcut { sequence: "Q"; context: Qt.ApplicationShortcut; onActivated: Qt.quit() }
    Shortcut { sequence: "Space"; context: Qt.ApplicationShortcut; onActivated: backend.primaryAction() }
    Shortcut { sequence: "R"; context: Qt.ApplicationShortcut; onActivated: backend.reset() }
    Shortcut { sequence: "S"; context: Qt.ApplicationShortcut; onActivated: backend.skip() }

    Item {
        anchors.fill: parent
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 24

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: win.phaseName(backend.phase)
                font.pixelSize: 22
                color: backend.phase === 0 ? theme.accent : "#9a9aa2"
            }

            Item {
                Layout.alignment: Qt.AlignHCenter
                width: Math.min(win.width, win.height - 200) * 0.75
                height: width

                Canvas {
                    id: ring
                    anchors.fill: parent
                    property real progress: backend.total > 0 ? backend.remaining / backend.total : 0
                    property string accent: theme.accent
                    onProgressChanged: requestPaint()
                    onAccentChanged: requestPaint()
                    onWidthChanged: requestPaint()
                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        const c = width / 2, r = c - 8
                        ctx.lineWidth = 8
                        ctx.strokeStyle = "#26262b"
                        ctx.beginPath(); ctx.arc(c, c, r, 0, Math.PI * 2); ctx.stroke()
                        ctx.strokeStyle = accent
                        ctx.lineCap = "round"
                        ctx.beginPath()
                        ctx.arc(c, c, r, -Math.PI / 2, -Math.PI / 2 + Math.PI * 2 * progress)
                        ctx.stroke()
                    }
                }

                Label {
                    anchors.centerIn: parent
                    text: win.clock(backend.remaining)
                    font.pixelSize: parent.width * 0.22
                    font.weight: Font.Light
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 12
                Button {
                    text: backend.running ? qsTr("Pause") : qsTr("Start")
                    highlighted: true
                    Material.background: theme.accent
                    Material.foreground: theme.accentForeground
                    onClicked: backend.primaryAction()
                }
                Button { text: qsTr("Reset"); flat: true; onClicked: backend.reset() }
                Button { text: qsTr("Skip"); flat: true; onClicked: backend.skip() }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: { translator.language; return qsTr("%n focus sessions done", "", backend.completed) }
                color: "#9a9aa2"
            }
        }

        Label {
            visible: backend.notifyError !== ""
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 12 }
            text: backend.notifyError
            color: "#ff8a80"
            font.pixelSize: 12
        }

        Rectangle {
            visible: win.helpVisible
            anchors.fill: parent
            color: "#e60e0e10"
            MouseArea { anchors.fill: parent; onClicked: win.helpVisible = false }
            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width - 40, 340)
                height: help.implicitHeight + 40
                radius: theme.radius
                color: "#1a1a1e"
                GridLayout {
                    id: help
                    anchors { fill: parent; margins: 20 }
                    columns: 2
                    columnSpacing: 20
                    Label { text: "Space" }  Label { text: qsTr("Start or pause") }
                    Label { text: "R" }      Label { text: qsTr("Reset the timer") }
                    Label { text: "S" }      Label { text: qsTr("Skip to the next phase") }
                    Label { text: "?" }      Label { text: qsTr("Show or hide this help") }
                    Label { text: "Q" }      Label { text: qsTr("Quit") }
                }
            }
        }
    }
}
