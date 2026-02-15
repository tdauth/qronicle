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
                
                // Die obere Zeile (ToolBar)
                ToolButton {
                    id: menuButton
                    text: "☰"
                    onClicked: mainMenu.open()

                    // Das eigentliche Dropdown-Menü
                    Menu {
                        id: mainMenu
                        y: menuButton.height // Erscheint direkt unter dem Button
                        
                        MenuItem {
                            text: qsTr("Settings")
                            onTriggered: console.log("Settings clicked")
                        }
                        
                        MenuSeparator { } // Ein horizontaler Trennstrich

                        MenuItem {
                            text: qsTr("Exit program")
                            onTriggered: Qt.quit()
                        }
                    }
                }
                
                // Suchfeld für FilePath
                TextField {
                    id: filePathSearch
                    // Bindung an das Model: Wird automatisch leer, wenn C++ das Signal sendet
                    text: chatModel.filterFilePath
                    placeholderText: qsTr("File Path...")
                    Layout.preferredWidth: 150
                    
                    // WICHTIG: activeFocus verhindert Endlosschleifen beim automatischen Leeren
                    onTextChanged: {
                        if (activeFocus) {
                            filePathTimer.restart()
                        }
                    }
                    Timer { 
                        id: filePathTimer
                        interval: 500
                        onTriggered: chatModel.filterFilePath = filePathSearch.text 
                    }
                }

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
                
                // Suchfeld für Sender
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
                
                // Suchfeld für Target
                TextField {
                    id: targetSearch
                    // Bindung an das Model: Wird automatisch leer, wenn C++ das Signal sendet
                    text: chatModel.filterTarget
                    placeholderText: qsTr("Receiver...")
                    Layout.preferredWidth: 150
                    
                    // WICHTIG: activeFocus verhindert Endlosschleifen beim automatischen Leeren
                    onTextChanged: {
                        if (activeFocus) {
                            targetTimer.restart()
                        }
                    }
                    Timer { 
                        id: targetTimer
                        interval: 500
                        onTriggered: chatModel.filterTarget = targetSearch.text 
                    }
                }
                
                // Suchfeld für Messenger
                TextField {
                    id: messengerSearch
                    text: chatModel.filterMessenger
                    placeholderText: qsTr("Messenger...")
                    Layout.preferredWidth: 150
                    
                    onTextChanged: {
                        if (activeFocus) {
                            messengerTimer.restart()
                        }
                    }
                    Timer { 
                        id: messengerTimer
                        interval: 500
                        onTriggered: chatModel.filterMessenger = messengerSearch.text 
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
                                    // Container für die Icons (Messenger & Protocol)
                                    Row {
                                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                        spacing: 6

                                        // --- MESSENGER ICON ---
                                        Item {
                                            width: 16
                                            height: 16
                                            // Nur anzeigen, wenn messenger gesetzt ist und sich vom Protokoll unterscheidet
                                            visible: typeof messenger !== "undefined" && messenger !== "" && 
                                                    messenger.toLowerCase() !== protocol.toLowerCase()

                                            Image {
                                                id: messengerIcon
                                                anchors.fill: parent
                                                source: `qrc:/icons/${messenger}`
                                                fillMode: Image.PreserveAspectFit
                                                onStatusChanged: if (status === Image.Error) visible = false
                                            }

                                            TextEdit {
                                                anchors.centerIn: parent
                                                text: `[${messenger}]`
                                                visible: messengerIcon.status === Image.Error
                                                font.pointSize: 7
                                                color: "#888"
                                                readOnly: true
                                                selectByMouse: true
                                            }

                                            MouseArea {
                                                id: messengerMouseArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                ToolTip {
                                                    visible: messengerMouseArea.containsMouse
                                                    text: qsTr("Messenger: %1").arg(messenger)
                                                    delay: 500
                                                    // FIX: Binding Loop verhindern durch explizite Zuweisung
                                                    contentWidth: implicitContentWidth 
                                                }
                                            }
                                        }

                                        // --- PROTOCOL ICON ---
                                        Item {
                                            width: 16
                                            height: 16

                                            Image {
                                                id: protocolIcon
                                                anchors.fill: parent
                                                source: `qrc:/icons/${protocol}`
                                                fillMode: Image.PreserveAspectFit
                                                onStatusChanged: if (status === Image.Error) visible = false
                                            }

                                            TextEdit {
                                                anchors.centerIn: parent
                                                text: `[${protocol}]`
                                                visible: protocolIcon.status === Image.Error
                                                font.pointSize: 7
                                                color: "#888"
                                                readOnly: true
                                                selectByMouse: true
                                            }
                                            
                                            MouseArea {
                                                id: protocolMouseArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                ToolTip {
                                                    visible: protocolMouseArea.containsMouse
                                                    text: qsTr("Protocol: %1").arg(protocol)
                                                    delay: 500
                                                    // FIX: Binding Loop verhindern durch explizite Zuweisung
                                                    contentWidth: implicitContentWidth
                                                }
                                            }
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
