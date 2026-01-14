#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>
// 引入所有子模块头文件
#include "loginwidget.h"
#include "scoreinputwidget.h"
#include "scorestatwidget.h"
#include "scorechartwidget.h"

// 前置声明UI类
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造/析构函数
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 核心函数：设置登录用户类型（控制模块权限）
    void setUserType(const QString& userType);

private slots:
    // 菜单栏动作槽函数
    void on_actionQuit_triggered();       // 退出程序
    void on_actionAbout_triggered();      // 关于信息

private:
    // 成员变量
    Ui::MainWindow *ui;
    QString m_currentUser;                // 当前登录用户名
    QString m_userType;                   // 用户类型（admin/normal）
    // 子模块指针（统一管理）
    ScoreInputWidget *m_inputWidget;
    ScoreStatWidget *m_statWidget;
    ScoreChartWidget *m_chartWidget;
};

#endif // MAINWINDOW_H
