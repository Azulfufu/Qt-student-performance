#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include "dbmanager.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    // 登录成功信号：传递用户类型+账号
    void loginSuccess(QString userType, QString username);

private slots:
    void on_btnLogin_clicked();  // 登录按钮点击
    void on_btnCancel_clicked(); // 取消按钮点击

private:
    Ui::LoginWidget *ui;
    // 验证账号密码
    bool verifyUser(QString username, QString password);
};

#endif // LOGINWIDGET_H
