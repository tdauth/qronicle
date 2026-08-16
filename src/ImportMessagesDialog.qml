import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root
    title: qsTr("Import Messages")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 350

    function openWithModel(optionsList) {
        modeComboBox.model = optionsList
        modeComboBox.currentIndex = 0
        root.open()
    }

    signal dialogAccepted(string folderPath, string selectedOption)

    Component.onCompleted: {
        var okButton = root.standardButton(Dialog.Ok)
        if (okButton) {
            okButton.enabled = Qt.binding(function() { return pathField.text !== "" })
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select messages directory")
        onAccepted: pathField.text = folderDialog.selectedFolder
    }

    onAccepted: {
        root.dialogAccepted(pathField.text, modeComboBox.currentText)
    }

    ColumnLayout {
        spacing: 15
        width: 300

        Label {
            text: qsTr("Folder Location:")
            font.bold: true
        }
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: pathField
                placeholderText: qsTr("No folder selected...")
                Layout.fillWidth: true
                readOnly: true
            }
            Button {
                text: qsTr("Browse...")
                onClicked: folderDialog.open()
            }
        }

        Label {
            text: qsTr("Messenger:")
            font.bold: true
        }

        ComboBox {
            id: modeComboBox
            Layout.fillWidth: true
        }
    }
}
