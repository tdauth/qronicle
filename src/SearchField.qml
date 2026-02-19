pragma NativeMethodBehavior: AcceptThisObject
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextField {
    id: control
    placeholderText: qsTr("Find...")
    Layout.fillWidth: true

    // Properties für die Flexibilität
    property var allItems: []
    property var filteredItems: []
    property string targetProperty: "" // Welches Feld im chatModel gefiltert wird
    property var fetchFunction: null   // Die Funktion zum Laden der Daten (z.B. getAllMessengers)

    // Signal zur Anwendung der Auswahl
    signal selectionMade(string value)

    onActiveFocusChanged: {
        if (activeFocus && fetchFunction) {
            allItems = fetchFunction.call(fetchFunction)
            filteredItems = allItems
        }
    }

    onTextChanged: {
        if (activeFocus) {
            searchTimer.restart()
            filteredItems = allItems.filter(item => {
                let val = String(item || "");
                return val.toLowerCase().includes(text.toLowerCase())
            })
        }
    }

    onPressed: if (filteredItems.length > 0) popup.open()

    // Das Popup wird nun über das Overlay gesteuert (verhindert Abschneiden)
    Popup {
        id: popup
        y: parent.height
        width: parent.width
        
        visible: control.activeFocus && control.filteredItems.length > 0 && control.text.length > 0
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        contentItem: ListView {
            //required property TextField control
            
            implicitHeight: Math.min(contentHeight, 200)
            model: control.filteredItems
            clip: true
            delegate: ItemDelegate {
                width: parent.width
                text: modelData
                onClicked: {
                    control.text = modelData
                    control.selectionMade(modelData)
                    popup.close()
                }
            }
        }
    }

    Timer {
        id: searchTimer
        interval: 500
        onTriggered: {
            if (targetProperty !== "") {
                chatModel[targetProperty] = control.text
            }
        }
    }
}
