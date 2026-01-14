#include "scorestatwidget.h"
#include "ui_ScoreStatWidget.h" // 必须包含UI生成的头文件（与UI文件名一致）

// 构造函数：初始化UI + 加载数据 + 初始化Model
ScoreStatWidget::ScoreStatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreStatWidget) // UI类名与头文件前置声明一致
    , m_relModel(nullptr)
    , m_proxyModel(nullptr)
{
    ui->setupUi(this); // 初始化UI控件

    // 初始化Model/View架构
    initModel();

    // 加载筛选下拉框（班级/课程）
    loadFilterOptions();

    // 初始化统计标签
    ui->labAvg->setText("平均分：--");
    ui->labMax->setText("最高分：--");
    ui->labMin->setText("最低分：--");
}

// 析构函数：释放Model和UI资源
ScoreStatWidget::~ScoreStatWidget()
{
    delete m_relModel;
    delete m_proxyModel;
    delete ui;
}

// 初始化Model/View：关联成绩表+学生表+课程表
void ScoreStatWidget::initModel()
{
    // 1. 创建关联模型：关联scores表与students/courses表
    m_relModel = new QSqlRelationalTableModel(this);
    m_relModel->setTable("scores"); // 主表为成绩表

    // 关联字段：scores.student_id → students.student_name（显示学生姓名）
    m_relModel->setRelation(1, QSqlRelation("students", "student_id", "student_name"));
    // 关联字段：scores.course_id → courses.course_name（显示课程名称）
    m_relModel->setRelation(2, QSqlRelation("courses", "course_id", "course_name"));
    // 关联字段：scores.student_id → students.class_name（显示班级名称）
    m_relModel->setRelation(1, QSqlRelation("students", "student_id", "class_name"));

    // 设置编辑策略：仅读取（统计模块不允许编辑）
    m_relModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // 查询数据
    m_relModel->select();

    // 2. 创建代理模型：用于筛选数据（不修改原模型）
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_relModel); // 关联到源模型
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 筛选不区分大小写

    // 3. 将代理模型绑定到TableView
    ui->tableView->setModel(m_proxyModel);
    ui->tableView->setSortingEnabled(true); // 允许点击表头排序
    ui->tableView->resizeColumnsToContents(); // 自适应列宽
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
}

// 加载筛选下拉框数据：班级+课程
void ScoreStatWidget::loadFilterOptions()
{
    // ===== 加载班级列表 =====
    QString sqlClass = "SELECT DISTINCT class_name FROM students ORDER BY class_name";
    QSqlQuery queryClass = DBManager::getInstance().execQuery(sqlClass);

    ui->cbxClass->clear();
    ui->cbxClass->addItem("全部"); // 默认选项：显示所有班级
    while (queryClass.next()) {
        ui->cbxClass->addItem(queryClass.value(0).toString());
    }

    // ===== 加载课程列表 =====
    QString sqlCourse = "SELECT DISTINCT course_name FROM courses ORDER BY course_name";
    QSqlQuery queryCourse = DBManager::getInstance().execQuery(sqlCourse);

    ui->cbxCourse->clear();
    ui->cbxCourse->addItem("全部"); // 默认选项：显示所有课程
    while (queryCourse.next()) {
        ui->cbxCourse->addItem(queryCourse.value(0).toString());
    }

    // 空数据提示
    if (ui->cbxClass->count() == 1) { // 只有"全部"选项
        QMessageBox::information(this, "提示", "数据库中暂无班级数据！");
    }
    if (ui->cbxCourse->count() == 1) { // 只有"全部"选项
        QMessageBox::information(this, "提示", "数据库中暂无课程数据！");
    }
}

// 执行数据筛选：根据班级+课程筛选
void ScoreStatWidget::filterData()
{
    QString className = ui->cbxClass->currentText();
    QString courseName = ui->cbxCourse->currentText();

    // 构建筛选条件（SQL WHERE语法）
    QString filterStr;
    if (className != "全部") {
        filterStr += QString("class_name = '%1'").arg(className);
    }
    if (courseName != "全部") {
        if (!filterStr.isEmpty()) filterStr += " AND ";
        filterStr += QString("course_name = '%1'").arg(courseName);
    }

    // 设置筛选条件并刷新
    m_proxyModel->setFilterFixedString(filterStr);
    ui->tableView->resizeColumnsToContents(); // 重新自适应列宽

    // 筛选后统计成绩
    statScores();
}
