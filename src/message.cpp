#include "message.hpp"

namespace qronicle {

Message::Message() {
}

void Message::setFilePath(const QString &filePath) {
    m_filePath = filePath;
}

QString Message::filePath() const {
    return m_filePath;
}

void Message::setLineNumber(qint64 lineNumber) {
    m_lineNumber = lineNumber;
}

qint64 Message::lineNumber() const {
    return m_lineNumber;
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
}

QString Message::content() const {
    return m_content;
}

void Message::setContentHtml(const QString &contentHtml) {
    m_contentHtml = contentHtml;
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

void Message::setMessenger(const QString &messenger) {
    m_messenger = messenger;
}

QString Message::messenger() const {
    return m_messenger;
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
