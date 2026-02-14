#include <QRegularExpression>

#include "message.hpp"

namespace chronicle {
    
Message::Message() {
}
        
void Message::setSource(const QString &source) {
    m_source = source;
}

QString Message::source() const {
    return m_source;
}

void Message::setSourceNick(const QString &sourceNick) {
    m_sourceNick = sourceNick;
}

QString Message::sourceNick() const {
    return m_sourceNick;
}

void Message::setDestination(const QString &destination) {
    m_destination = destination;
}

QString Message::destination() const {
    return m_destination;
}

void Message::setDestinationNick(const QString &destinationNick) {
    m_destinationNick = destinationNick;
}

QString Message::destinationNick() const {
    return m_destinationNick;
}

void Message::setContent(const QString &content) {
    m_content = content;
    m_contentHtml = content.toHtmlEscaped();
    
    // 1. Nur bearbeiten, wenn überhaupt ein Link-Protokoll vorkommt
    if (content.contains("http://") || content.contains("https://")) {
        static QRegularExpression urlRegex(R"((https?:\/\/[^\s\n\r]+))");
        
        m_contentHtml = m_contentHtml.replace(urlRegex, R"(<a href="\1">\1</a>)");
    }
}

QString Message::content() const {
    return m_content;
}

QString Message::contentHtml() const {
    return m_contentHtml;
}

void Message::setTimestamp(const QDateTime &timestamp) {
    m_timestamp = timestamp;
}

QDateTime Message::timestamp() const {
    return m_timestamp;
}

void Message::setProtocol(const QString &protocol) {
    m_protocol = protocol;
}

QString Message::protocol() const {
    return m_protocol;
}

void Message::setStatus(const QString &status) {
    m_status = status;
}

QString Message::status() const {
    return m_status;
}
    
    
}
