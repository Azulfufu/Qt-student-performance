#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "loginwidget.h"
#include "dbmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 初始化数据库
    if (!DBManager::getInstance().initDB("E:/Qt/lab/lab1.db")) {
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
