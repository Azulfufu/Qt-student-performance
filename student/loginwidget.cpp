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


