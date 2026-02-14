import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 700
    title: qsTr("Chronicle - Kopete History Viewer")

    // Design-Konstanten
    readonly property color colorBgChat: "#e5ddd5"

    // Hauptcontainer (ersetzt SplitView)
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // --- 1. GANZ OBEN: Suchbereich ---
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#f8f9fa" // Leichtes Grau vom Rest abgesetzt

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 10

                Label {
                    text: "Search:"
                    font.pixelSize: 13
                    font.bold: true
                    color: "#555"
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Filter messages or contacts...")
                    font.pixelSize: 13
                    selectByMouse: true
                    
                    Timer {
                        id: searchDelayTimer
                        interval: 300 // Millisekunden warten
                        repeat: false
                        onTriggered: {
                            chatModel.setFilterFixedString(searchField.text)
                        }
                    }
                    
                    background: Rectangle {
                        radius: 4
                        border.color: searchField.activeFocus ? "#007bff" : "#ccc"
                        color: "white"
                    }

                    onTextChanged: {
                        searchDelayTimer.restart() // Timer bei jedem Tastendruck neu starten
                    }
                }
                
                Button {
                    text: qsTr("Clear")
                    visible: searchField.text !== ""
                    onClicked: searchField.text = ""
                }
            }

            // Trennlinie zum nächsten Bereich
            Rectangle {
                width: parent.width
                height: 1
                color: "#ddd"
                anchors.bottom: parent.bottom
            }
        }

        // Chat-Hintergrund für die Liste
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: colorBgChat

            ListView {
                id: chatListView
                anchors.fill: parent
                anchors.margins: 15
                model: chatModel
                spacing: 12
                clip: true
                reuseItems: true 
                cacheBuffer: 3000
                
                // Der Scrollbalken
                ScrollBar.vertical: ScrollBar {
                    id: vBar
                    policy: ScrollBar.AlwaysOn // Erzwingt die dauerhafte Sichtbarkeit
                    active: true               // Sorgt dafür, dass er nicht halbtransparent wird
                    
                    // Optional: Optisches Tuning, damit er nicht über dem Text liegt
                    parent: chatListView.parent 
                    anchors.top: chatListView.top
                    anchors.bottom: chatListView.bottom
                    anchors.right: chatListView.right
                }

                delegate: Column {
                    width: chatListView.width - 30
                    spacing: 4

                    // Die Sprechblase
                    Rectangle {
                        width: parent.width
                        height: innerCol.implicitHeight + 16
                        color: "#ffffff"
                        radius: 6
                        border.color: "#ddd"

                        Column {
                            id: innerCol
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4
                            
                            // Zeile 1: Sender (links) & Zeit (rechts)
                            RowLayout {
                                width: parent.width
                                Text {
                                    text: qsTr("<b>%1</b> (%2)")
                                        .arg(sourceNick || qsTr("Unknown"))
                                        .arg(sourceId)
                                    font.pointSize: 9
                                    color: "#2c3e50"
                                    textFormat: Text.StyledText
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: time
                                    font.pointSize: 8
                                    color: "#666"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                                }
                            }
                            
                            // Zeile 2: Empfänger (links) & Protokoll (rechts)
                            RowLayout {
                                width: parent.width
                                Text {
                                    text: qsTr("<b>To:</b> %1 (%2)")
                                            .arg(targetNick || qsTr("Unknown"))
                                            .arg(targetId)
                                    font.pointSize: 9
                                    color: "#7f8c8d"
                                    textFormat: Text.StyledText
                                    Layout.fillWidth: true
                                }
                                Text {
                                    text: `[${protocol}]`
                                    font.pointSize: 7
                                    color: "#888"
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                            
                            // Trennlinie
                            Rectangle {
                                width: parent.width
                                height: 1
                                color: "#eee"
                                visible: messageText !== ""
                                Layout.topMargin: 2
                                Layout.bottomMargin: 2
                            }
                            
                            // Nachrichtentext
                            Text {
                                id: msgContent
                                text: messageText
                                width: parent.width
                                wrapMode: Text.WordWrap
                                font.pointSize: 10
                                textFormat: Text.StyledText
                                
                                onLinkActivated: (link) => {
                                    Qt.openUrlExternally(link)
                                }
                                
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.NoButton
                                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.DefaultCursor
                                }
                            }
                        }
                    }
                }
                
                onCountChanged: chatListView.positionViewAtEnd()
            }
        }
    }
}
