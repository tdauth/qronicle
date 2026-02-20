import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextField {
    id: control
    placeholderText: qsTr("Find...")
    Layout.fillWidth: true

    property var allItems: []
    property var filteredItems: []
    property string targetProperty: "" 
    property var fetchFunction: null   
    
    // NEU: Flag, um das automatische Wiederöffnen zu verhindern
    property bool isSelecting: false

    signal selectionMade(string value)

    function updateVisibility() {
        // Nur öffnen, wenn wir NICHT gerade aktiv ein Element auswählen
        if (!isSelecting && activeFocus && filteredItems.length > 0) {
            popup.open()
        } else {
            popup.close()
        }
    }

    onActiveFocusChanged: {
        if (activeFocus && fetchFunction) {
            isSelecting = false // Reset beim neuen Fokus
            allItems = fetchFunction.call(fetchFunction)
            filteredItems = allItems
            updateVisibility()
        }
    }

    onTextChanged: {
        if (activeFocus && !isSelecting) {
            searchTimer.restart()
            filteredItems = allItems.filter(item => {
                let val = String(item || "");
                return val.toLowerCase().includes(text.toLowerCase())
            })
            updateVisibility()
        }
    }

    Popup {
        id: popup
        y: parent.height
        width: parent.width
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, 200)
            model: control.filteredItems
            clip: true
            delegate: ItemDelegate {
                width: ListView.view.width
                text: modelData
                onClicked: {
                    control.isSelecting = true // 1. Sperre setzen
                    control.text = modelData   // 2. Text ändern (triggert onTextChanged)
                    control.selectionMade(modelData)
                    popup.close()              // 3. Popup schließen
                    
                    // Fokus wegzunehmen ist die sicherste Methode, um das Popup zu schließen
                    control.focus = false 
                    
                    // Sperre kurz verzögert lösen, falls der User direkt wieder reinklickt
                    control.isSelecting = false
                }
            }
        }
    }

    Timer {
        id: searchTimer
        interval: 500
        onTriggered: {
            if (targetProperty !== "" && typeof chatModel !== "undefined") {
                chatModel[targetProperty] = control.text
            }
        }
    }
}
