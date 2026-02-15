#include <QRegularExpression>

#include "messenger.hpp"

namespace qronicle {

QString Messenger::formatHtml(const QString &msg) {
    // 1. Früher Ausstieg: Wenn kein "http" drin ist, gibt es nichts zu tun.
    // Das spart bei 90% der Nachrichten den teuren Regex-Check.
    if (!msg.contains(u"://")) {
        return msg;
    }

    static QRegularExpression urlRegex(R"(([a-zA-Z]+://[^\s\n\r]+))");
    
    QString result;
    // Reserviere im Voraus Platz, um Reallokationen zu minimieren
    result.reserve(msg.length() + 32); 

    int lastPos = 0;
    auto it = urlRegex.globalMatch(msg);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        
        // Text vor dem Link hinzufügen
        result.append(QStringView{msg}.mid(lastPos, match.capturedStart() - lastPos));
        
        // Den Link als HTML-Tag einbauen
        QStringView url = match.capturedView(1);
        result.append(u"<a href=\"");
        result.append(url);
        result.append(u"\">");
        result.append(url);
        result.append(u"</a>");
        
        lastPos = match.capturedEnd();
    }
    
    // Restlichen Text nach dem letzten Treffer hinzufügen
    result.append(QStringView{msg}.mid(lastPos));
    
    return result;
}

}
