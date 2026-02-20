import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextField {
    id: control
    
    property var selectedDateTime: new Date()
    property var internalDate: new Date()
    property string dateTimeFormat: "dd.MM.yyyy HH:mm"

    placeholderText: qsTr("Choose date time...")
    readOnly: true
    text: selectedDateTime ? selectedDateTime.toLocaleString(Qt.locale(), dateTimeFormat) : ""

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            // Aktuellen Wert sicher in internen Status kopieren
            control.internalDate = new Date(control.selectedDateTime.getTime())
            hourSpin.value = control.internalDate.getHours()
            minSpin.value = control.internalDate.getMinutes()
            datePopup.open()
        }
    }

    Popup {
        id: datePopup
        y: control.height + 2
        width: 450 
        padding: 15
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle { 
            color: "white"
            border.color: "#bbb"
            radius: 4 
            layer.enabled: true
        }

        RowLayout {
            anchors.fill: parent
            spacing: 20

            // --- Left side: Calendar ---
            ColumnLayout {
                spacing: 5
                RowLayout {
                    Layout.fillWidth: true
                    Button { text: "<"; flat: true; onClicked: grid.month === 0 ? (grid.month = 11, grid.year--) : grid.month-- }
                    Label { 
                        text: Qt.locale().monthName(grid.month) + " " + grid.year
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.bold: true 
                    }
                    Button { text: ">"; flat: true; onClicked: grid.month === 11 ? (grid.month = 0, grid.year++) : grid.month++ }
                }

                DayOfWeekRow { 
                    locale: grid.locale
                    Layout.fillWidth: true
                    delegate: Label { 
                        text: model.shortName
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        color: "#666"
                    } 
                }

                MonthGrid {
                    id: grid
                    Layout.preferredWidth: 230
                    Layout.preferredHeight: 180
                    month: control.internalDate.getMonth()
                    year: control.internalDate.getFullYear()

                    delegate: Button {
                        implicitWidth: 32; implicitHeight: 32
                        flat: true
                        padding: 0
                        contentItem: Label {
                            text: model.day
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: highlighted ? control.palette.highlight : "transparent"
                            radius: 16
                            opacity: highlighted ? 0.3 : 1
                        }
                        highlighted: model.day === control.internalDate.getDate() && 
                                     model.month === control.internalDate.getMonth() && 
                                     model.year === control.internalDate.getFullYear()
                        onClicked: {
                            let d = new Date(control.internalDate.getTime())
                            d.setFullYear(model.year, model.month, model.day)
                            control.internalDate = d
                        }
                    }
                }
            }

            // Vertical Separator
            Rectangle { Layout.fillHeight: true; width: 1; color: "#eee" }

            // --- Right side: Time & OK ---
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: 150
                spacing: 15

                Label { text: qsTr("Select Time"); font.bold: true; Layout.alignment: Qt.AlignHCenter }

                GridLayout {
                    columns: 2
                    rowSpacing: 12
                    columnSpacing: 8
                    Layout.alignment: Qt.AlignHCenter

                    Label { text: qsTr("Hour:") }
                    SpinBox {
                        id: hourSpin
                        from: 0; to: 23; editable: true
                        Layout.preferredWidth: 100 // Breit genug für 2 Ziffern + Buttons
                        textFromValue: (value) => String(value).padStart(2, '0')
                        valueFromText: (text) => parseInt(text, 10)
                        
                        contentItem: TextInput {
                            text: hourSpin.textFromValue(hourSpin.value, hourSpin.locale)
                            font: hourSpin.font
                            color: hourSpin.palette.text
                            selectionColor: hourSpin.palette.highlight
                            selectedTextColor: hourSpin.palette.highlightedText
                            horizontalAlignment: Qt.AlignHCenter
                            verticalAlignment: Qt.AlignVCenter
                            readOnly: !hourSpin.editable
                            validator: hourSpin.validator
                            inputMethodHints: Qt.ImhDigitsOnly
                        }
                    }

                    Label { text: qsTr("Min:") }
                    SpinBox {
                        id: minSpin
                        from: 0; to: 59; editable: true
                        Layout.preferredWidth: 100
                        textFromValue: (value) => String(value).padStart(2, '0')
                        valueFromText: (text) => parseInt(text, 10)
                        
                        contentItem: TextInput {
                            text: minSpin.textFromValue(minSpin.value, minSpin.locale)
                            font: minSpin.font
                            color: minSpin.palette.text
                            selectionColor: minSpin.palette.highlight
                            selectedTextColor: minSpin.palette.highlightedText
                            horizontalAlignment: Qt.AlignHCenter
                            verticalAlignment: Qt.AlignVCenter
                            readOnly: !minSpin.editable
                            validator: minSpin.validator
                            inputMethodHints: Qt.ImhDigitsOnly
                        }
                    }
                }

                Item { Layout.fillHeight: true } // Spacer

                Button {
                    text: qsTr("Apply")
                    Layout.fillWidth: true
                    highlighted: true
                    onClicked: {
                        let finalD = new Date(control.internalDate.getTime())
                        finalD.setHours(hourSpin.value, minSpin.value, 0)
                        control.selectedDateTime = finalD
                        datePopup.close()
                    }
                }
            }
        }
    }
}
