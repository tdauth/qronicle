#ifndef CHRONICLE_MESSAGE_HPP
#define CHRONICLE_MESSAGE_HPP

#include <QString>
#include <QDateTime>

namespace chronicle {
    
class Message {
    
    public:
        Message();
        
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
        QString contentHtml() const;
        void setTimestamp(const QDateTime &timestamp);
        QDateTime timestamp() const;
        void setProtocol(const QString &protocol);
        QString protocol() const;
        void setStatus(const QString &status);
        QString status() const;
    
    private:
        QString m_source;      // Wer hat gesendet (z. B. ICQ-Nummer)
        QString m_sourceNick; // Nickname von source
        QString m_destination; // Wer hat empfangen
        QString m_destinationNick; // Nickname von destination
        QString m_content;     // Die Nachricht selbst
        QString m_contentHtml; // URLs automatisch formatiert
        QDateTime m_timestamp; // Präziser Zeitpunkt
        QString m_protocol;    // z.B. "ICQ", "Jabber"
        QString m_status; // z. B. "Gelesen"
    
};

}

#endif
