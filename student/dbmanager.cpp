#include "dbmanager.h"
bool DBManager::initDB(const QString& dbPath)
{
    if (m_db.isOpen()) return true; // 避免重复连接
    // 初始化SQLite连接
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qCritical() << "数据库连接失败：" << m_db.lastError().text();
        return false;
    }
    qInfo() << "数据库连接成功！";
    return true;
}
