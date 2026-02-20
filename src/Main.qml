import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 700

    // Design-Konstanten
    readonly property color colorBgChat: "#e5ddd5"
    
    Action {
        id: openConfigAction
        text: qsTr("Open config folder")
        shortcut: "Ctrl+,"
        onTriggered: {
            let folderUrl = "file://" + appConfigPath;
            console.log("Open:", folderUrl);
            Qt.openUrlExternally(folderUrl);
        }
    }

    Action {
        id: aboutAction
        text: qsTr("About")
        shortcut: "F1"
        onTriggered: aboutDialog.open()
    }


    Action {
        id: quitAction
        text: qsTr("Exit")
        shortcut: StandardKey.Quit
        onTriggered: Qt.quit()
    }
    
    Dialog {
        id: aboutDialog
        title: qsTr("About")

        anchors.centerIn: parent
        width: 300
        height: 350
        modal: true
        standardButtons: Dialog.Ok

        contentItem: Column {
            spacing: 15
            topPadding: 10
            bottomPadding: 20

            Image {
                id: logo
                source: "qrc:/icons/qronicle"
                width: 80
                height: 80
                anchors.horizontalCenter: parent.horizontalCenter

                fillMode: Image.PreserveAspectFit
                smooth: true

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "#eeeeee"
                    border.width: 1
                    visible: logo.status === Image.Ready
                }
            }

            Label {
                text: qsTr("qronicle 1.0")
                font.pixelSize: 18
                font.bold: true
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                text: qsTr("Build date: %1\nCopyright © 2026 Tamino Dauth\nAlle rights reserved.").arg(Qt.formatDateTime(buildDateTime, "yyyy-MM-dd HH:mm"))
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                text: "<a href='https://github.com/tdauth/qronicle'>https://github.com/tdauth/qronicle</a>"
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }
    }

    // Hauptcontainer (ersetzt SplitView)
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- 1. Die ToolBar (Feste Höhe) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#f8f9fa" // Leichtes Grau vom Rest abgesetzt

            RowLayout {
                spacing: 10
                anchors.fill: parent

                // toolbar
                ToolButton {
                    id: menuButton
                    text: "☰"
                    onClicked: mainMenu.open()

                    // Das eigentliche Dropdown-Menü
                    Menu {
                        id: mainMenu
                        implicitWidth: Math.max(300, contentItem.implicitWidth + 40) 
                        y: menuButton.height // Erscheint direkt unter dem Button
                        
                        MenuItem {
                            action: openConfigAction
                        }

                        MenuItem {
                            action: aboutAction
                        }

                        MenuSeparator { }

                        MenuItem {
                            action: quitAction
                        }
                    }
                }

                ToolButton {
                    id: filterButton
                    text: qsTr("Filter")

                    Shortcut {
                        id: filterShortcut
                        sequence: StandardKey.Find
                        onActivated: filterButton.toggle()
                    }

                    icon.name: "view-filter"
                    icon.source: "qrc:/icons/fallback-filter.svg"

                    checkable: true
                    checked: false // Standardmäßig aus
                    onClicked: filterPanel.visible = checked

                    contentItem: Label {
                        text: filterButton.text + (filterButton.checked ? " ▴" : " ▾") + " (" + filterShortcut.nativeText + ")"
                        font: filterButton.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Item { Layout.fillWidth: true } // Spacer
            }
        }

        // 2. Das Filter-Panel (wird durch den ToolButton gesteuert)
        Rectangle {
            id: filterPanel
            visible: filterButton.checked // Direkt an den Button-Zustand gebunden
            Layout.fillWidth: true
            Layout.preferredHeight: filterGrid.implicitHeight + 20
            color: Qt.darker("#f5f5f5", 1.02) // Dezenter Kontrast

            GridLayout {
                id: filterGrid
                anchors.fill: parent
                anchors.margins: 10
                columns: 2
                columnSpacing: 8
                rowSpacing: 8

                // --- Reihe 1: Reset-Button (klein) + 2 Textfelder ---
                Action {
                    id: resetFiltersAction
                    text: qsTr("Reset")
                    icon.name: "edit-clear"

                    onTriggered: {
                        chatModel.filterParticipant = ""
                        chatModel.filterSender = ""
                        chatModel.filterTarget = ""
                        chatModel.filterMessenger = ""
                        chatModel.filterProtocol = ""
                        chatModel.filterFilePath = ""
                        chatModel.filterMessage = ""
                        chatModel.filterFrom = chatModel.getFrom()
                        chatModel.filterTo = chatModel.getTo()

                        participantSearch.text = ""
                        senderSearch.text = ""
                        receiverSearch.text = ""
                        messengerSearch.text = ""
                        protocolSearch.text = ""
                        filePathSearch.text = ""
                        messageSearch.text = ""
                        
                        fromSearch.selectedDateTime = chatModel.filterFrom
                        toSearch.selectedDateTime = chatModel.filterTo
                    }
                }

                Button {
                    id: resetFiltersButton
                    action: resetFiltersAction

                    Layout.fillWidth: false
                    Layout.preferredWidth: 120
                    Layout.alignment: Qt.AlignLeft

                    flat: true
                }
                
                SearchField {
                    id: participantSearch
                    placeholderText: qsTr("Participant...")
                    targetProperty: "filterParticipant"
                    fetchFunction: chatModel.getAllNickNames
                    text: chatModel.filterParticipant
                    onSelectionMade: (val) => chatModel.filterParticipant = val
                }
                
                SearchField {
                    id: senderSearch
                    placeholderText: qsTr("Sender...")
                    targetProperty: "filterSender"
                    fetchFunction: chatModel.getAllNickNames
                    text: chatModel.filterSender
                    onSelectionMade: (val) => chatModel.filterSender = val
                }
                
                SearchField {
                    id: receiverSearch
                    placeholderText: qsTr("Receiver...")
                    targetProperty: "filterTarget"
                    fetchFunction: chatModel.getAllNickNames
                    text: chatModel.filterTarget
                    onSelectionMade: (val) => chatModel.filterTarget = val
                }

                SearchField {
                    id: messengerSearch
                    placeholderText: qsTr("Messenger...")
                    targetProperty: "filterMessenger"
                    fetchFunction: chatModel.getAllMessengers
                    text: chatModel.filterMessenger
                    onSelectionMade: (val) => chatModel.filterMessenger = val
                }

                SearchField {
                    id: protocolSearch
                    placeholderText: qsTr("Protocol...")
                    targetProperty: "filterProtocol"
                    fetchFunction: chatModel.getAllProtocols
                    text: chatModel.filterProtocol
                    onSelectionMade: (val) => chatModel.filterProtocol = val
                }

                TextField {
                    id: filePathSearch
                    placeholderText: qsTr("File Path...")
                    text: chatModel.filterFilePath
                    Layout.fillWidth: true
                    onTextChanged: if (activeFocus) filePathTimer.restart()
                    Timer { id: filePathTimer; interval: 500; onTriggered: chatModel.filterFilePath = parent.text }
                }

                TextField {
                    id: messageSearch
                    placeholderText: qsTr("Search Message Content...")
                    text: chatModel.filterMessage
                    Layout.fillWidth: true
                    onTextChanged: if (activeFocus) msgTimer.restart()
                    Timer { id: msgTimer; interval: 500; onTriggered: chatModel.filterMessage = parent.text }
                }
                
                DateTimePicker {
                    id: fromSearch
                    selectedDateTime: chatModel.filterFrom
                    Layout.fillWidth: true
                    Layout.columnSpan: 1
                    onSelectedDateTimeChanged: fromTimer.restart()
                    Timer { id: fromTimer; interval: 500; onTriggered: chatModel.filterFrom = fromSearch.selectedDateTime }
                }
                
                DateTimePicker {
                    id: toSearch
                    selectedDateTime: chatModel.filterTo
                    Layout.fillWidth: true
                    Layout.columnSpan: 1
                    onSelectedDateTimeChanged: {
                        console.log("DEBUG: Picker Signal gefeuert!", selectedDateTime)
                        toTimer.restart()
                    }
                    Timer { id: toTimer; interval: 500; onTriggered: chatModel.filterTo = toSearch.selectedDateTime }
                }
            }

            Rectangle {
                width: parent.width; height: 1
                color: "#ccc";
                anchors.bottom: parent.bottom
            }
        }

        // chat background for list
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

                property bool initialScrollDone: false

                onCountChanged: {
                    if (!initialScrollDone && chatModel.totalCount > 0) {
                        initialScrollDone = true

                        // 1. Erster Versuch: Zum aktuell bekannten Ende
                        chatListView.positionViewAtIndex(count - 1, ListView.End)

                        // 2. Erzwungener Versuch: Nach einer kurzen Verzögerung zum ECHTEN Ende
                        // Das gibt der SQLite-Engine Zeit, das 'fetchMore' intern zu verarbeiten
                        var scrollTimer = Qt.createQmlObject('import QtQuick; Timer { interval: 100; repeat: false }', chatListView)
                        scrollTimer.triggered.connect(function() {
                            // Wir springen zum absoluten Maximum aus deiner SQL-Abfrage
                            chatListView.positionViewAtIndex(chatModel.totalCount - 1, ListView.End)
                            scrollTimer.destroy()
                        })
                        scrollTimer.start()
                    }
                }

                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Home) {
                        positionViewAtBeginning()
                        event.accepted = true
                    } else if (event.key === Qt.Key_End) {
                        positionViewAtEnd()
                        event.accepted = true
                    }
                }

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

                delegate: Rectangle {
                    id: speechBubble
                    width: chatListView.width - vBar.width

                    height: innerCol.implicitHeight + 16
                    radius: 6
                    
                    color: out ? "#e1ffc7" : "#ffffff"
                    
                    border.width: 1
                    border.color: out ? "#c7e3ae" : "#dddddd"
                    
                    // pass model properties into delegate
                    required property string sourceNick
                    required property string sourceId
                    required property string sourceAvatar
                    required property string targetNick
                    required property string targetId
                    required property string targetAvatar
                    required property string messageText
                    required property string messenger
                    required property string protocol
                    required property string filePath
                    required property string lineNumber
                    required property string time
                    required property bool out

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

                        RowLayout {
                            width: parent.width

                            Row {
                                width: parent.width
                                spacing: 8  // Abstand in Pixeln zwischen den Elementen

                                Image {
                                    fillMode: Image.PreserveAspectCrop
                                    source: sourceAvatar ? "image://avatars/" + sourceAvatar : ""
                                    sourceSize.width: 16  // WICHTIG: Teilt dem Provider die 'requestedSize' mit
                                    sourceSize.height: 16
                                    asynchronous: true    // Erlaubt das Laden im Hintergrund
                                }

                                TextEdit {
                                    text: qsTr("From: %1 (%2)")
                                        .arg(speechBubble.sourceNick || qsTr("Unknown"))
                                        .arg(speechBubble.sourceId)
                                    font.pointSize: 9
                                    color: "#2c3e50"
                                    textFormat: Text.StyledText
                                    Layout.fillWidth: true
                                    selectByMouse: true
                                    selectionColor: "#3498db"
                                    persistentSelection: true
                                    readOnly: true // Verhindert Bearbeitung beim Klicken auf den Link
                                }

                                Image {
                                    fillMode: Image.PreserveAspectCrop
                                    source: targetAvatar ? "image://avatars/" + speechBubble.targetAvatar : ""
                                    sourceSize.width: 16  // WICHTIG: Teilt dem Provider die 'requestedSize' mit
                                    sourceSize.height: 16
                                    asynchronous: true    // Erlaubt das Laden im Hintergrund
                                }

                                TextEdit {
                                    text: qsTr("To: %1 (%2)")
                                            .arg(speechBubble.targetNick || qsTr("Unknown"))
                                            .arg(speechBubble.targetId)
                                    font.pointSize: 9
                                    color: "#7f8c8d"
                                    textFormat: Text.StyledText
                                    Layout.fillWidth: true
                                    readOnly: true
                                    selectByMouse: true
                                    selectionColor: "#3498db"
                                    persistentSelection: true
                                }

                                Item {
                                    width: 16
                                    height: 16

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

                                        Popup {
                                            id: messengerPopup
                                            // Sichtbarkeit steuern
                                            visible: messengerMouseArea.containsMouse

                                            // Verhindert, dass das Popup den Fokus stiehlt oder schließt
                                            focus: false
                                            closePolicy: Popup.NoAutoClose

                                            // Positionierung: Etwas unterhalb der Maus oder des Elements
                                            y: parent.height + 5
                                            x: 0

                                            background: Rectangle {
                                                color: "#ffffff"
                                                border.color: "#bbbbbb"
                                                radius: 2
                                                // Schatten-Effekt (optional, für ToolTip-Optik)
                                                layer.enabled: true
                                            }

                                            contentItem: Label {
                                                text: qsTr("Messenger: %1").arg(messenger)
                                                font.pointSize: 8
                                                color: "#333"
                                                padding: 5
                                            }
                                        }
                                    }
                                }

                                Item {
                                    width: 16
                                    height: 16
                                    // Nur anzeigen, wenn protocol gesetzt ist und sich vom messenger unterscheidet
                                    visible: typeof protocol !== "undefined" && protocol !== "" &&
                                            protocol.toLowerCase() !== messenger.toLowerCase()

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

                                        Popup {
                                            id: protocolPopup
                                            // Sichtbarkeit steuern
                                            visible: protocolMouseArea.containsMouse

                                            // Verhindert, dass das Popup den Fokus stiehlt oder schließt
                                            focus: false
                                            closePolicy: Popup.NoAutoClose

                                            // Positionierung: Etwas unterhalb der Maus oder des Elements
                                            y: parent.height + 5
                                            x: 0

                                            background: Rectangle {
                                                color: "#ffffff"
                                                border.color: "#bbbbbb"
                                                radius: 2
                                                // Schatten-Effekt (optional, für ToolTip-Optik)
                                                layer.enabled: true
                                            }

                                            contentItem: Label {
                                                text: qsTr("Protocol: %1").arg(protocol)
                                                font.pointSize: 8
                                                color: "#333"
                                                padding: 5
                                            }
                                        }
                                    }
                                }

                                // Dateilink (URL auf filePath, zeigt nur Dateinamen)
                                TextEdit {
                                    id: fileText

                                    // WICHTIG: Erst das Format, dann der Text
                                    textFormat: Text.RichText // Versuche RichText statt StyledText, falls es Probleme gibt

                                    readonly property string maskedFilePath: filePath.replace(/%/g, "%25") // Fixes file names from Psi like at_icq.jabber.fh%2dstralsund.de.history
                                    readonly property string fullUrl: "file://" + maskedFilePath + (lineNumber > 0 ? "#" + lineNumber : "")
                                    readonly property string fileName: filePath.split('/').pop()

                                    // Layout-Integration
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.preferredWidth: contentWidth // Nutzt die tatsächliche Textbreite

                                    // Styling & Inhalt
                                    // Inline-Style für die Farbe, da linkColor in TextEdit nicht existiert
                                    text: "<a href='" + fullUrl + "' style='color:#3498db; text-decoration:none;'>" + fileName + "</a>"
                                    font.pointSize: 9
                                    color: "#3498db"

                                    readOnly: true
                                    selectByMouse: true

                                    // Link-Klick Logik
                                    onLinkActivated: (link) => {
                                        console.log("Opening link:", link);
                                        Qt.openUrlExternally(link);
                                    }

                                    // Kontextmenü
                                    Menu {
                                        id: contextMenuCopyLink
                                        MenuItem {
                                            text: qsTr("Copy Link Address")
                                            onTriggered: fileText.copyToClipboard(fileText.fullUrl)
                                        }
                                        MenuItem {
                                            text: qsTr("Copy File Path")
                                            onTriggered: fileText.copyToClipboard(filePath)
                                        }
                                    }

                                    // Helferfunktion (Nutzt das interne Clipboard-System von TextEdit)
                                    function copyToClipboard(txt) {
                                        tempCopyEdit.text = txt;
                                        tempCopyEdit.selectAll();
                                        tempCopyEdit.copy();
                                    }

                                    // Unsichtbarer Helfer für Clipboard
                                    TextEdit { id: tempCopyEdit; visible: false }

                                    // MouseArea für Cursor & Rechtsklick
                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        acceptedButtons: Qt.RightButton
                                        // Verweist auf hoveredLink des Elternelements (TextEdit)
                                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor

                                        onClicked: (mouse) => {
                                            if (mouse.button === Qt.RightButton) {
                                                contextMenuCopyLink.popup();
                                            }
                                        }
                                    }
                                }

                                TextEdit {
                                    text: (time !== undefined && time !== null && time !== "") 
                                            ? Qt.formatDateTime(time, Qt.DefaultLocaleShortDate) 
                                            : qsTr("unknown")
                                    font.pointSize: 8
                                    color: "#666"
                                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                                    selectByMouse: true
                                    selectionColor: "#3498db"
                                    persistentSelection: true
                                    readOnly: true
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
                            horizontalAlignment: out ? TextEdit.AlignRight : TextEdit.AlignLeft

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

                    Menu {
                        id: contextMenu

                        MenuItem {
                            text: qsTr("Copy Message")
                            onTriggered: {
                                // 'messageText' ist der Name deiner Role aus C++
                                chatModel.copyToClipboard(model.messageText)
                            }
                        }

                        MenuItem {
                            text: qsTr("Jump to this message (clear filters)")
                            onTriggered: {
                                let modelRef = chatModel
                                let savedId = model.messageId
                                console.log("Save ID " + savedId)
                                resetFiltersAction.trigger() // Filter im C++ Model löschen

                                Qt.callLater(() => {
                                    // Erst jetzt ist der Proxy wieder "voll"
                                    let targetIdx = modelRef.findIndexById(savedId)
                                    console.log("Target Index " + targetIdx);
                                    if (targetIdx !== -1) {
                                        chatListView.positionViewAtIndex(targetIdx, ListView.Center)
                                        chatListView.currentIndex = targetIdx
                                    }
                                })
                            }
                        }
                    }
                }
            }
        }

        // Footer shows count and date range of all messages.
        Rectangle {
            Layout.fillWidth: true
            height: 25
            color: "#f0f0f0"
            border.color: "#ccc"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10

                // Left Side: Count
                Label {
                    text: qsTr("Messages: %1 / %2").arg(chatModel.filteredCount).arg(chatModel.totalCount)
                    font.pixelSize: 11
                    color: "#666"
                }

                Item { Layout.fillWidth: true } // Platzhalter schiebt Rest nach rechts

                // Center: Time span
                Label {
                    // Zeigt z.B. "Period: 01.01.2023 - 15.02.2026"
                    text: qsTr("Period: %1").arg(chatModel.dateRange)
                    font.pixelSize: 11
                    color: "#666"
                    visible: chatModel.totalCount > 0
                }

                Item { Layout.fillWidth: true } // Zweiter Platzhalter für Zentrierung

                // Right Side: Status
                Label {
                    text: chatModel.filteredCount === chatModel.totalCount ?
                        qsTr("All data loaded") :
                        qsTr("Filtered")
                    font.pixelSize: 11
                    font.italic: true
                    color: "#999"
                }
            }
        }
    }
}
