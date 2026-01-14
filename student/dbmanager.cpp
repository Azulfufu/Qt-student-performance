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

QSqlQuery DBManager::execQuery(const QString& sql)
{
    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        qCritical() << "查询失败：" << sql << " 错误：" << query.lastError().text();
    }
    return query;
}

bool DBManager::execNonQuery(const QString& sql)
{
    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        qCritical() << "执行失败：" << sql << " 错误：" << query.lastError().text();
        return false;
    }
    return true;
}

QString DBManager::encryptPassword(QString password)
{
    // MD5加密：输入明文密码，返回32位加密字符串
    QByteArray byteArray = password.toUtf8();
    byteArray = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    return byteArray.toHex();
}


