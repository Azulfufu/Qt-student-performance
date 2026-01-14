#include "mainwindow.h"
#include "ui_MainWindow.h"

// 构造函数：初始化UI + 加载子模块 + 权限控制
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentUser("")
    , m_userType("")
    , m_inputWidget(nullptr)
    , m_statWidget(nullptr)
    , m_chartWidget(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("学生成绩管理系统 v1.0"); // 设置窗口标题
    this->setMinimumSize(800, 600); // 设置最小尺寸，避免窗口过小

    // ========== 1. 初始化所有子模块 ==========
    m_inputWidget = new ScoreInputWidget();   // 成绩录入模块
    m_statWidget = new ScoreStatWidget();     // 成绩统计模块
    m_chartWidget = new ScoreChartWidget();   // 成绩图表模块

    // ========== 2. 将子模块添加到TabWidget ==========
    ui->tabWidget->addTab(m_inputWidget, "成绩录入");   // 第一个Tab
    ui->tabWidget->addTab(m_statWidget, "成绩统计");     // 第二个Tab
    ui->tabWidget->addTab(m_chartWidget, "成绩图表");     // 第三个Tab

    // ========== 3. 初始化状态栏 ==========
    ui->statusBar->showMessage(QString("系统就绪 - 当前时间：%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
}

// 析构函数：释放所有子模块和UI资源
MainWindow::~MainWindow()
{
    // 释放子模块
    delete m_inputWidget;
    delete m_statWidget;
    delete m_chartWidget;
    // 释放UI
    delete ui;
}

// 核心：设置用户类型，控制模块权限
void MainWindow::setUserType(const QString& userType)
{
    m_userType = userType;
    // ========== 权限控制规则 ==========
    // - admin（管理员）：可访问所有模块（录入+统计+图表）
    // - normal（普通用户）：仅可访问统计+图表，隐藏录入模块
    if (userType == "normal") {
        // 移除成绩录入Tab（索引0）
        ui->tabWidget->removeTab(0);
        // 更新状态栏：提示普通用户权限
        ui->statusBar->showMessage(QString("当前登录：普通用户 - 权限限制：不可录入成绩"));
    } else if (userType == "admin") {
        // 更新状态栏：提示管理员权限
        ui->statusBar->showMessage(QString("当前登录：管理员 - 权限：可访问所有模块"));
    }
}

// ========== 菜单栏槽函数：退出程序 ==========
void MainWindow::on_actionQuit_triggered()
{
    // 确认退出
    int ret = QMessageBox::question(this, "退出", "确定要退出学生成绩管理系统吗？",
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        this->close(); // 关闭主窗口
    }
}

// ========== 菜单栏槽函数：关于信息 ==========
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于",
                       "学生成绩管理系统 v1.0\n"
                       "基于Qt 6.5.3开发\n"
                       "功能：成绩录入、统计、图表展示");
}
