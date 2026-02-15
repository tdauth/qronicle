#ifndef CHRONICLE_SETTINGS_MANAGER_HPP
#define CHRONICLE_SETTINGS_MANAGER_HPP

#include <QObject>
#include <QString>
#include <QSettings>
#include <QVariantMap>

namespace qronicle {
    
class SettingsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap settingsMap READ settingsMap NOTIFY settingsMapChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr) : QObject(parent) {
        loadGroup("General");
    }

    Q_INVOKABLE void loadGroup(QString groupName) {
        m_currentGroup = groupName;
        m_settings.beginGroup(groupName);
        m_map.clear();
        for (const QString &key : m_settings.childKeys()) {
            m_map[key] = m_settings.value(key);
        }
        m_settings.endGroup();
        emit settingsMapChanged();
    }

    Q_INVOKABLE void setSetting(QString key, QVariant value) {
        m_settings.beginGroup(m_currentGroup);
        m_settings.setValue(key, value);
        m_settings.endGroup();
        m_map[key] = value;
    }

    QVariantMap settingsMap() const { return m_map; }

signals:
    void settingsMapChanged();

private:
    QSettings m_settings;
    QVariantMap m_map;
    QString m_currentGroup;
};

}

#endif
