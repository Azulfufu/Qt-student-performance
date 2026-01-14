#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("学生成绩系统 - 登录");
    ui->labError->setVisible(false); // 隐藏错误提示
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

// 验证账号密码逻辑
bool LoginWidget::verifyUser(QString username, QString password)
{
    if (username.isEmpty() || password.isEmpty()) {
        ui->labError->setText("账号/密码不能为空！");
        ui->labError->setVisible(true);
        return false;
    }

    // 查询用户信息（加密后对比）
    QString encryptPwd = DBManager::encryptPassword(password);
    QString sql = QString("SELECT password, user_type FROM users WHERE username = '%1'").arg(username);
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (!query.next()) { // 账号不存在
        ui->labError->setText("账号不存在！");
        ui->labError->setVisible(true);
        return false;
    }

    if (query.value(0).toString() != encryptPwd) { // 密码错误
        ui->labError->setText("密码错误！");
        ui->labError->setVisible(true);
        return false;
    }
    return true;
}

// 取消按钮点击
void LoginWidget::on_btnCancel_clicked()
{
    this->close();
    qApp->exit(); // 退出程序
}

// 登录按钮点击
void LoginWidget::on_btnLogin_clicked()
{
    QString username = ui->leUsername->text().trimmed();
    QString password = ui->lePassword->text().trimmed();

    if (verifyUser(username, password)) {
        // 获取用户类型
        QString sql = QString("SELECT user_type FROM users WHERE username = '%1'").arg(username);
        QSqlQuery query = DBManager::getInstance().execQuery(sql);
        query.next();
        QString userType = query.value(0).toString();

        emit loginSuccess(userType, username); // 发送登录成功信号
        this->close();
    }
}
