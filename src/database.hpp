#ifndef CHRONICLE_DATABASE_HPP
#define CHRONICLE_DATABASE_HPP

#include <QSqlDatabase>
#include <QMap>

#include "messenger.hpp"

namespace qronicle {
    
class Database {
    public:
        Database();
        ~Database();
        
        void removeDatabaseFile();
        void saveMessages(const Messenger::Messages &messages);
        void applyAliases(QMap<QString, QString> &&aliases);
        
        QSqlDatabase db() const { return m_db; }
        
    private:
        void initDb();
        
        QSqlDatabase m_db;
};

}

#endif
