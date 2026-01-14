#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include "mainwindow.h"
#include "loginwidget.h"
#include "dbmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 检查数据库文件是否存在
    QString dbPath = "studentdb.db";
    QFileInfo dbFile(dbPath);

    if (!dbFile.exists()) {
        QMessageBox::critical(nullptr, "错误",
                              QString("数据库文件不存在！\n路径：%1\n当前目录：%2")
                                  .arg(dbPath)
                                  .arg(QDir::currentPath()));
        return -1;
    }

    // 1. 初始化数据库
    if (!DBManager::getInstance().initDB(dbPath)) {
        QMessageBox::critical(nullptr, "错误", "数据库连接失败！");
        return -1;
    }

    // 2. 显示登录窗口
    LoginWidget loginWidget;
    MainWindow mainWindow;

    // 3. 登录成功后显示主窗口
    QObject::connect(&loginWidget, &LoginWidget::loginSuccess, [&](QString userType, QString username){
        mainWindow.setUserType(userType);
        mainWindow.setWindowTitle(QString("学生成绩系统 - 当前用户：%1（%2）").arg(username).arg(userType));
        mainWindow.show();
    });

    loginWidget.show();
    return a.exec();
}
