import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 700
    
    // Füge diese Aliase oben hinzu:
    property alias msgField: messageSearch
    property alias nickField: nickSearch
    property alias protField: protocolSearch

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
                spacing: 10
                anchors.fill: parent

                // Suchfeld für Nachrichtentext
                TextField {
                    id: messageSearch
                    // Das Feld zeigt IMMER das an, was im Model steht
                    text: chatModel.filterMessage 
                    placeholderText: qsTr("Message...")
                    Layout.fillWidth: true
                    
                    // Wenn der User tippt, wird das Model aktualisiert
                    onTextChanged: {
                        if (activeFocus) { // Nur wenn der User tippt, nicht beim automatischen Reset
                            msgTimer.restart()
                        }
                    }

                    Timer {
                        id: msgTimer
                        interval: 500
                        onTriggered: chatModel.filterMessage = messageSearch.text
                    }
                }
                // Suchfeld für Nickname
                TextField {
                    id: nickSearch
                    // Bindung an das Model: Wird automatisch leer, wenn C++ das Signal sendet
                    text: chatModel.filterNick
                    placeholderText: qsTr("Sender...")
                    Layout.preferredWidth: 150
                    
                    // WICHTIG: activeFocus verhindert Endlosschleifen beim automatischen Leeren
                    onTextChanged: {
                        if (activeFocus) {
                            nickTimer.restart()
                        }
                    }
                    Timer { 
                        id: nickTimer
                        interval: 500
                        onTriggered: chatModel.filterNick = nickSearch.text 
                    }
                }

                // Suchfeld für Protocol
                TextField {
                    id: protocolSearch
                    text: chatModel.filterProtocol
                    placeholderText: qsTr("Protocol...")
                    Layout.preferredWidth: 150
                    
                    onTextChanged: {
                        if (activeFocus) {
                            protocolTimer.restart()
                        }
                    }
                    Timer { 
                        id: protocolTimer
                        interval: 500
                        onTriggered: chatModel.filterProtocol = protocolSearch.text 
                    }
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
                    id: messageDelegate
                    width: chatListView.width - 30
                    spacing: 4
                    
                    Row { // Äußere Reihe für Avatar + Sprechblase
                        spacing: 8
                        width: parent.width

                        // 1. SENDER AVATAR (Links)
                        Rectangle {
                            width: 36; height: 36
                            radius: 18
                            color: "#eee"
                            clip: true
                            visible: model.sourceAvatar && !model.sourceAvatar.isNull

                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                source: model.sourceAvatar ? "image://avatars/" + model.sourceAvatar : ""
                            }
                        }

                        // Die Sprechblase
                        Rectangle {
                            // Berechnet die Breite dynamisch: Gesamtbreite minus sichtbare Avatare
                            width: parent.width 
                                - (model.sourceAvatar ? 44 : 0) 
                                - (model.targetAvatar ? 44 : 0)
                            
                            height: innerCol.implicitHeight + 16
                            color: "#ffffff"
                            radius: 6
                            border.color: "#ddd"
                            
                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.RightButton
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        contextMenu.popup();
                                    }
                                }
                            }
                            

                            Column {
                                id: innerCol
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 4
                                
                                // Zeile 1: Sender (links) & Zeit (rechts)
                                RowLayout {
                                    width: parent.width
                                    TextEdit {
                                        text: qsTr("From: %1 (%2)")
                                            .arg(sourceNick || qsTr("Unknown"))
                                            .arg(sourceId)
                                        font.pointSize: 9
                                        color: "#2c3e50"
                                        textFormat: Text.StyledText
                                        Layout.fillWidth: true
                                        selectByMouse: true
                                        selectionColor: "#3498db"
                                        persistentSelection: true
                                    }
                                    TextEdit {
                                        text: time
                                        font.pointSize: 8
                                        color: "#666"
                                        Layout.alignment: Qt.AlignRight | Qt.AlignTop
                                        selectByMouse: true
                                        selectionColor: "#3498db"
                                        persistentSelection: true
                                    }
                                }
                                
                                // Zeile 2: Empfänger (links) & Protokoll (rechts)
                                RowLayout {
                                    width: parent.width
                                    TextEdit {
                                        text: qsTr("To: %1 (%2)")
                                                .arg(targetNick || qsTr("Unknown"))
                                                .arg(targetId)
                                        font.pointSize: 9
                                        color: "#7f8c8d"
                                        textFormat: Text.StyledText
                                        Layout.fillWidth: true
                                        readOnly: true
                                        selectByMouse: true
                                        selectionColor: "#3498db"
                                        persistentSelection: true
                                    }
                                    // Das Protokoll-Icon/Label rechts
                                    Item {
                                        Layout.preferredWidth: 16
                                        Layout.preferredHeight: 16
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                                        // Das Icon
                                        Image {
                                            id: protocolIcon
                                            anchors.fill: parent
                                            // Logik: Suche im Ordner "icons" nach "facebook.png", "skype.png" etc.
                                            source: `qrc:/icons/${protocol.toLowerCase()}`
                                            fillMode: Image.PreserveAspectFit
                                            
                                            // Falls das Icon nicht gefunden wird, zeigen wir den Text-Fallback
                                            onStatusChanged: if (status === Image.Error) visible = false
                                        }

                                        // Fallback: Text, falls kein Icon da ist
                                        TextEdit {
                                            anchors.centerIn: parent
                                            text: `[${protocol}]`
                                            visible: protocolIcon.status === Image.Error
                                            font.pointSize: 7
                                            color: "#888"
                                            readOnly: true
                                            selectByMouse: true
                                            selectionColor: "#3498db"
                                            persistentSelection: true
                                        }
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
                                TextEdit {
                                    id: msgContent
                                    text: messageText
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    font.pointSize: 10
                                    textFormat: Text.RichText
                                    readOnly: true
                                    selectByMouse: true
                                    selectionColor: "#3498db"
                                    persistentSelection: true
                                    
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
                        
                        // 3. EMPFÄNGER AVATAR (Rechts)
                        Rectangle {
                            width: 36; height: 36
                            radius: 18
                            color: "#eee"
                            clip: true
                            // Sichtbar nur, wenn targetAvatar Daten hat
                            visible: model.targetAvatar && !model.targetAvatar.isNull

                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                source: model.targetAvatar ? "image://avatars/" + model.targetAvatar : ""
                            }
                        }
                    }

                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("Jump to this message (clear filters)")
                            onTriggered: {
                                // 1. C++ aufrufen (Filter leeren, Ziel-Index holen)
                                let targetIdx = chatModel.getUnfilteredIndex(index)
                                
                                if (targetIdx !== -1) {
                                    // 2. Die ListView über die 'view'-Eigenschaft des Delegates finden
                                    // 'messageDelegate' ist die ID deiner Column im Delegate
                                    let listView = messageDelegate.ListView.view
                                    
                                    if (listView) {
                                        // 3. Scrollen und Index setzen
                                        listView.positionViewAtIndex(targetIdx, ListView.Center)
                                        listView.currentIndex = targetIdx
                                    }
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
