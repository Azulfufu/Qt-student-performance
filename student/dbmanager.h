#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

class DBManager
{
public:
    // 单例模式，全局唯一数据库连接
    static DBManager& getInstance() {
        static DBManager instance;
        return instance;
    }

    // 初始化数据库连接
    bool initDB(const QString& dbPath);

    // 执行查询语句（返回结果）
    QSqlQuery execQuery(const QString& sql);

    // 执行增删改语句（返回成功/失败）
    bool execNonQuery(const QString& sql);

    // 检查连接状态
    bool isConnected() { return m_db.isOpen(); }

    // 密码MD5加密（工具函数）
    static QString encryptPassword(QString password);

    // 新增：获取最后一次数据库错误信息（解决未定义报错）
    QString getLastError() const { return m_db.lastError().text(); }

private:
    // 私有构造/析构，禁止外部实例化
    DBManager() {}
    ~DBManager() { m_db.close(); }

    // 禁止拷贝
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
