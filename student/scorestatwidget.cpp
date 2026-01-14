#include "scorestatwidget.h"
#include "ui_ScoreStatWidget.h"
#include <QRegularExpression>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError> // 修复QSqlError
#include "dbmanager.h" // 修复DBManager

// 构造函数
ScoreStatWidget::ScoreStatWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ScoreStatWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("成绩统计");

    // 初始化模型
    m_relModel = new QSqlRelationalTableModel(this);
    m_relModel->setTable("scores");
    // 关联学生表（假设student_id关联students表的student_id，显示姓名和班级）
    m_relModel->setRelation(m_relModel->fieldIndex("student_id"),
                            QSqlRelation("students", "student_id", "student_name,class_name"));
    // 关联科目表（course_id关联courses表的course_id，显示科目名称）
    m_relModel->setRelation(m_relModel->fieldIndex("course_id"),
                            QSqlRelation("courses", "course_id", "course_name"));
    m_relModel->select();

    // 初始化代理模型（关键：启用筛选）
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_relModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 忽略大小写

    // 新增：班级下拉框选择变化时触发筛选
    connect(ui->cbxClass, &QComboBox::currentIndexChanged, this, &ScoreStatWidget::filterData);
    // 新增：科目下拉框选择变化时触发筛选
    connect(ui->cbxCourse, &QComboBox::currentIndexChanged, this, &ScoreStatWidget::filterData);

    // 设置表格模型
    ui->tableView->setModel(m_proxyModel);
    ui->tableView->resizeColumnsToContents();

    // 加载班级和科目下拉框（补充你的原有代码）
    loadClassList();
    loadCourseList();
}
// 析构函数
ScoreStatWidget::~ScoreStatWidget()
{
    delete m_relModel;
    delete m_proxyModel;
    delete ui;
}

// 初始化Model/View：核心修复关联配置
void ScoreStatWidget::initModel()
{
    // 1. 创建关联模型：主表为scores
    m_relModel = new QSqlRelationalTableModel(this);
    m_relModel->setTable("scores");

    // ===== 关键修复：正确配置关联（一个字段只能关联一次）=====
    // 先获取scores表的字段索引（避免硬编码列号）
    int studentIdCol = m_relModel->fieldIndex("student_id");
    int courseIdCol = m_relModel->fieldIndex("course_id");
    int scoreCol = m_relModel->fieldIndex("score");

    // 关联1：student_id → students.student_name（显示学生姓名）
    if (studentIdCol != -1) {
        m_relModel->setRelation(studentIdCol, QSqlRelation("students", "student_id", "student_name"));
    }
    // 关联2：course_id → courses.course_name（显示课程名称）
    if (courseIdCol != -1) {
        m_relModel->setRelation(courseIdCol, QSqlRelation("courses", "course_id", "course_name"));
    }

    // 设置编辑策略：仅读取
    m_relModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // 设置表头名称（友好显示）
    m_relModel->setHeaderData(studentIdCol, Qt::Horizontal, "学生ID");
    m_relModel->setHeaderData(courseIdCol, Qt::Horizontal, "课程名称");
    m_relModel->setHeaderData(scoreCol, Qt::Horizontal, "成绩");
    m_relModel->setHeaderData(m_relModel->fieldIndex("exam_date"), Qt::Horizontal, "考试日期");

    // 查询数据（必须调用select()加载数据）
    if (!m_relModel->select()) {
        QMessageBox::critical(this, "错误", "加载成绩数据失败：" + m_relModel->lastError().text());
        return;
    }

    // 2. 创建代理模型：修复筛选逻辑
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_relModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // 关键：设置筛选模式为正则表达式（支持复杂条件）
    m_proxyModel->setFilterRole(Qt::DisplayRole);

    // 3. 绑定到TableView
    ui->tableView->setModel(m_proxyModel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 隐藏不需要的列（如score_id）
    int scoreIdCol = m_relModel->fieldIndex("score_id");
    if (scoreIdCol != -1) {
        ui->tableView->hideColumn(scoreIdCol);
    }
}

// 加载筛选下拉框数据
void ScoreStatWidget::loadFilterOptions()
{
    // ===== 加载班级列表 =====
    QString sqlClass = "SELECT DISTINCT class_name FROM students WHERE class_name IS NOT NULL ORDER BY class_name";
    QSqlQuery queryClass = DBManager::getInstance().execQuery(sqlClass);

    ui->cbxClass->clear();
    ui->cbxClass->addItem("全部");
    while (queryClass.next()) {
        QString className = queryClass.value(0).toString().trimmed();
        if (!className.isEmpty()) {
            ui->cbxClass->addItem(className);
        }
    }

    // ===== 加载课程列表 =====
    QString sqlCourse = "SELECT DISTINCT course_name FROM courses WHERE course_name IS NOT NULL ORDER BY course_name";
    QSqlQuery queryCourse = DBManager::getInstance().execQuery(sqlCourse);

    ui->cbxCourse->clear();
    ui->cbxCourse->addItem("全部");
    while (queryCourse.next()) {
        QString courseName = queryCourse.value(0).toString().trimmed();
        if (!courseName.isEmpty()) {
            ui->cbxCourse->addItem(courseName);
        }
    }

    // 空数据提示
    if (ui->cbxClass->count() == 1) {
        QMessageBox::information(this, "提示", "数据库中暂无班级数据！");
    }
    if (ui->cbxCourse->count() == 1) {
        QMessageBox::information(this, "提示", "数据库中暂无课程数据！");
    }
}

// 核心修复：筛选数据（适配代理模型+关联字段）
// 核心修复：筛选数据（适配代理模型+关联字段）
void ScoreStatWidget::filterData()
{
    QString targetClass = ui->cbxClass->currentText().trimmed();
    QString targetCourse = ui->cbxCourse->currentText().trimmed();

    // 创建一个组合筛选字符串
    QString filterString;

    // 构建班级筛选部分
    if (targetClass != "全部") {
        // 注意：由于是关联表，class_name来自students表
        filterString += QString("class_name LIKE '%%1%'").arg(targetClass);
    }

    // 构建课程筛选部分
    if (targetCourse != "全部") {
        if (!filterString.isEmpty()) {
            filterString += " AND ";
        }
        // 注意：由于是关联表，course_name来自courses表
        filterString += QString("course_name LIKE '%%1%'").arg(targetCourse);
    }

    // 应用筛选到源模型
    if (!filterString.isEmpty()) {
        m_relModel->setFilter(filterString);
    } else {
        m_relModel->setFilter(""); // 清空筛选
    }

    // 重新加载数据
    if (!m_relModel->select()) {
        QMessageBox::critical(this, "错误", "筛选数据失败：" + m_relModel->lastError().text());
        return;
    }

    ui->tableView->resizeColumnsToContents();
    statScores();
}

// ========== 修复统计逻辑：基于筛选后的结果 ==========
void ScoreStatWidget::statScores()
{
    float avgScore = 0.0f;
    float maxScore = -1.0f;
    float minScore = 101.0f;
    int validCount = 0;

    // 成绩列索引：界面中“score”是第2列，索引为1
    int scoreCol = m_relModel->fieldIndex("score");

    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        QModelIndex scoreIndex = m_proxyModel->index(row, scoreCol);
        if (!scoreIndex.isValid()) continue;

        float score = m_proxyModel->data(scoreIndex).toFloat();
        if (score >= 0 && score <= 100) {
            avgScore += score;
            maxScore = qMax(maxScore, score);
            minScore = qMin(minScore, score);
            validCount++;
        }
    }

    if (validCount > 0) {
        avgScore /= validCount;
        ui->labAvg->setText(QString("平均分：%1").arg(avgScore, 0, 'f', 1));
        ui->labMax->setText(QString("最高分：%1").arg(maxScore));
        ui->labMin->setText(QString("最低分：%1").arg(minScore));
    } else {
        ui->labAvg->setText("平均分：--");
        ui->labMax->setText("最高分：--");
        ui->labMin->setText("最低分：--");
    }
}
// ========== 加载班级列表到下拉框 ==========
void ScoreStatWidget::loadClassList()
{
    ui->cbxClass->clear();
    ui->cbxClass->addItem("全部");

    // 查询所有班级（去重）
    QString sql = "SELECT DISTINCT class_name FROM students ORDER BY class_name";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "错误", "查询班级失败：" + query.lastError().text());
        return;
    }

    while (query.next()) {
        QString className = query.value(0).toString().trimmed();
        if (!className.isEmpty()) {
            ui->cbxClass->addItem(className);
        }
    }
}

// ========== 加载科目列表到下拉框 ==========
void ScoreStatWidget::loadCourseList()
{
    ui->cbxCourse->clear();
    ui->cbxCourse->addItem("全部");

    // 查询所有科目
    QString sql = "SELECT course_name FROM courses ORDER BY course_name";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "错误", "查询科目失败：" + query.lastError().text());
        return;
    }

    while (query.next()) {
        QString courseName = query.value(0).toString().trimmed();
        if (!courseName.isEmpty()) {
            ui->cbxCourse->addItem(courseName);
        }
    }
}
// 槽函数：班级下拉框变化
void ScoreStatWidget::on_cbxClass_currentTextChanged(const QString &/*arg1*/)
{
    filterData();
}

// 槽函数：课程下拉框变化
void ScoreStatWidget::on_cbxCourse_currentTextChanged(const QString &/*arg1*/)
{
    filterData();
}
