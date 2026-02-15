#ifndef CHRONICLE_MESSAGE_HPP
#define CHRONICLE_MESSAGE_HPP

#include <QString>
#include <QByteArray>
#include <QDateTime>

namespace qronicle {
    
class Message {
    
    public:
        Message();
        
        void setFilePath(const QString &filePath);
        QString filePath() const;
        void setLineNumber(qint64 lineNumber);
        qint64 lineNumber() const;
        void setSource(const QString &source);
        QString source() const;
        void setSourceNick(const QString &sourceNick);
        QString sourceNick() const;
        void setDestination(const QString &destination);
        QString destination() const;
        void setDestinationNick(const QString &destinationNick);
        QString destinationNick() const;
        void setContent(const QString &content);
        QString content() const;
        void setContentHtml(const QString &contentHtml);
        QString contentHtml() const;
        void setTimestamp(const QDateTime &timestamp);
        QDateTime timestamp() const;
        void setMessenger(const QString &messenger);
        QString messenger() const;
        void setProtocol(const QString &protocol);
        QString protocol() const;
        void setStatus(const QString &status);
        QString status() const;
    
    private:
        QString m_filePath;
        qint64 m_lineNumber;
        QString m_source;      // Wer hat gesendet (z. B. ICQ-Nummer)
        QString m_sourceNick; // Nickname von source
        QString m_destination; // Wer hat empfangen
        QString m_destinationNick; // Nickname von destination
        QString m_content;     // Die Nachricht selbst
        QString m_contentHtml; // URLs automatisch formatiert
        QDateTime m_timestamp; // Präziser Zeitpunkt
        QString m_messenger;    // z.B. "Kopete", "Skype"
        QString m_protocol;    // z.B. "ICQ", "Jabber"
        QString m_status; // z. B. "Gelesen"
};

// Ignore filePath, sourceNick, destinationNick, contentHtml, messenger and status.
inline bool operator==(const Message &m1, const Message &m2) {
    // Bleibe bei Sekunden
    return m1.timestamp().toSecsSinceEpoch() == m2.timestamp().toSecsSinceEpoch() &&
           m1.source() == m2.source() &&
           m1.destination() == m2.destination() &&
           m1.content() == m2.content();
}

inline size_t qHash(const Message &key, size_t seed = 0) {
    return ::qHash(key.content(), seed) ^ 
           ::qHash(key.source(), seed) ^ 
           ::qHash(key.destination(), seed) ^ 
           ::qHash(key.timestamp().toSecsSinceEpoch(), seed); 
}

}

#endif
