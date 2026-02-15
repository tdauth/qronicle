import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#f0f0f0"
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20

        Label { text: qsTr("Settings"); font.bold: true; font.pixelSize: 18 }

        Repeater {
            model: Object.keys(settingsManager.settingsMap)
            delegate: RowLayout {
                Layout.fillWidth: true
                Label { text: modelData; Layout.preferredWidth: 100 }
                TextField {
                    text: settingsManager.settingsMap[modelData]
                    Layout.fillWidth: true
                    onEditingFinished: settingsManager.setSetting(modelData, text)
                }
            }
        }
        
        Button {
            text: qsTr("Back")
            onClicked: settingsLoader.active = false // Schließt die Ansicht
        }
    }
}
