#include "scorestatwidget.h"
#include "ui_ScoreStatWidget.h"
#include <QRegularExpression>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDateTime>
#include <QDir>
#include <QColor>
#include <QFont>
#include <QAxObject>
#include <QVariant>
#include "dbmanager.h"

// 构造函数
ScoreStatWidget::ScoreStatWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ScoreStatWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("成绩统计");

    initModel();
    loadFilterOptions();

    connect(ui->cbxClass, &QComboBox::currentIndexChanged, this, &ScoreStatWidget::filterData);

    connect(ui->cbxCourse, &QComboBox::currentIndexChanged, this, &ScoreStatWidget::filterData);

    connect(ui->btnExportExcel, &QPushButton::clicked, this, &ScoreStatWidget::on_btnExportExcel_clicked);


    ui->tableView->resizeColumnsToContents();
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
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


    int studentIdCol = m_relModel->fieldIndex("student_id");
    int courseIdCol = m_relModel->fieldIndex("course_id");
    int scoreCol = m_relModel->fieldIndex("score");
    int examDateCol = m_relModel->fieldIndex("exam_date");


    if (studentIdCol != -1) {
        m_relModel->setRelation(studentIdCol, QSqlRelation("students", "student_id", "student_name"));
    }

    if (courseIdCol != -1) {
        m_relModel->setRelation(courseIdCol, QSqlRelation("courses", "course_id", "course_name"));
    }


    m_relModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    if (studentIdCol != -1)
        m_relModel->setHeaderData(studentIdCol, Qt::Horizontal, "学生姓名");
    if (courseIdCol != -1)
        m_relModel->setHeaderData(courseIdCol, Qt::Horizontal, "课程名称");
    if (scoreCol != -1)
        m_relModel->setHeaderData(scoreCol, Qt::Horizontal, "成绩");
    if (examDateCol != -1)
        m_relModel->setHeaderData(examDateCol, Qt::Horizontal, "考试日期");


    if (!m_relModel->select()) {
        QMessageBox::critical(this, "错误", "加载成绩数据失败：" + m_relModel->lastError().text());
        return;
    }


    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_relModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_proxyModel->setFilterRole(Qt::DisplayRole);


    ui->tableView->setModel(m_proxyModel);


    int scoreIdCol = m_relModel->fieldIndex("score_id");
    if (scoreIdCol != -1) {
        ui->tableView->hideColumn(scoreIdCol);
    }
}


void ScoreStatWidget::loadFilterOptions()
{

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
void ScoreStatWidget::filterData()
{
    QString targetClass = ui->cbxClass->currentText().trimmed();
    QString targetCourse = ui->cbxCourse->currentText().trimmed();

    QString filterString;

    // 班级筛选：明确指定scores.student_id关联students表
    if (targetClass != "全部") {
        filterString += QString("scores.student_id IN (SELECT student_id FROM students WHERE class_name LIKE '%%1%')")
        .arg(targetClass);
    }

    // 课程筛选：明确指定scores.course_id关联courses表
    if (targetCourse != "全部") {
        if (!filterString.isEmpty()) {
            filterString += " AND ";
        }
        filterString += QString("scores.course_id IN (SELECT course_id FROM courses WHERE course_name LIKE '%%1%')")
                            .arg(targetCourse);
    }

    m_relModel->setFilter(filterString);
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

    // 成绩列索引：从模型获取，避免硬编码
    int scoreCol = m_relModel->fieldIndex("score");
    if (scoreCol == -1) {
        ui->labAvg->setText("平均分：--");
        ui->labMax->setText("最高分：--");
        ui->labMin->setText("最低分：--");
        return;
    }

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

void ScoreStatWidget::loadClassList()
{
    loadFilterOptions();
}

void ScoreStatWidget::loadCourseList()
{
    loadFilterOptions();
}

bool ScoreStatWidget::exportToExcel(const QString &filePath)
{
    if (m_proxyModel->rowCount() == 0) {
        QMessageBox::warning(this, "提示", "暂无数据可导出！");
        return false;
    }

#ifdef Q_OS_WIN
    system("taskkill /f /im EXCEL.EXE >nul 2>&1");
#endif

    QAxObject *excel = new QAxObject("Excel.Application", this);
    if (!excel || excel->isNull()) {
        delete excel;
        QMessageBox::critical(this, "错误", "无法创建Excel对象，请确保已安装Microsoft Excel！");
        return false;
    }

    excel->setProperty("DisplayAlerts", false);
    excel->setProperty("Visible", false);
    excel->setProperty("AutoSave", false);

    QAxObject *workBooks = excel->querySubObject("Workbooks");
    QAxObject *workBook = workBooks->querySubObject("Add");
    if (!workBook) {
        excel->dynamicCall("Quit()");
        delete workBook;
        delete workBooks;
        delete excel;
        QMessageBox::critical(this, "错误", "无法创建Excel工作簿！");
        return false;
    }

    QAxObject *workSheet = workBook->querySubObject("Worksheets(int)", 1);
    if (!workSheet) {
        workBook->dynamicCall("Close(false)");
        excel->dynamicCall("Quit()");
        delete workSheet;
        delete workBook;
        delete workBooks;
        delete excel;
        QMessageBox::critical(this, "错误", "无法获取Excel工作表！");
        return false;
    }

    try {
        // ========== 1. 写入表头（用Cells定位，彻底避免Range拼接错误） ==========
        QStringList headerList = {"学生姓名", "课程名称", "成绩", "考试日期"};
        for (int col = 0; col < headerList.size(); col++) {
            QAxObject *cell = workSheet->querySubObject("Cells(int, int)", 1, col + 1);
            if (!cell) continue;

            cell->dynamicCall("SetValue(const QVariant&)", headerList[col]);
            cell->querySubObject("Font")->setProperty("Bold", true);
            cell->querySubObject("Interior")->setProperty("Color", QColor(200, 200, 200).rgb());
            cell->querySubObject("Borders")->setProperty("LineStyle", 1);
            cell->setProperty("HorizontalAlignment", -4108);

            delete cell;
        }

        // ========== 2. 写入数据（同样用Cells定位）==========
        int rowCount = m_proxyModel->rowCount();

        // 关键修复：使用代理模型，而不是原始模型
        // 获取代理模型中的列索引（这些是显示给用户看到的列）
        int proxyModelColumnCount = m_proxyModel->columnCount();
        int studentNameCol = -1;
        int courseNameCol = -1;
        int scoreCol = -1;
        int examDateCol = -1;

        // 根据表头文字找到对应的列索引
        for (int col = 0; col < proxyModelColumnCount; col++) {
            QString header = m_proxyModel->headerData(col, Qt::Horizontal).toString();
            if (header == "学生姓名") {
                studentNameCol = col;
            } else if (header == "课程名称") {
                courseNameCol = col;
            } else if (header == "成绩") {
                scoreCol = col;
            } else if (header == "考试日期") {
                examDateCol = col;
            }
        }

        // 验证所有必需的列都已找到
        if (studentNameCol == -1 || courseNameCol == -1 || scoreCol == -1 || examDateCol == -1) {
            QMessageBox::critical(this, "错误", "无法确定数据列的位置！");
            workBook->dynamicCall("Close(false)");
            excel->dynamicCall("Quit()");
            delete workSheet;
            delete workBook;
            delete workBooks;
            delete excel;
            return false;
        }

        for (int row = 0; row < rowCount; row++) {
            // 学生姓名（第1列）
            QAxObject *cellA = workSheet->querySubObject("Cells(int, int)", row + 2, 1);
            cellA->dynamicCall("SetValue(const QVariant&)", m_proxyModel->data(m_proxyModel->index(row, studentNameCol), Qt::DisplayRole));
            cellA->querySubObject("Borders")->setProperty("LineStyle", 1);
            cellA->setProperty("HorizontalAlignment", -4108);
            delete cellA;

            // 课程名称（第2列）
            QAxObject *cellB = workSheet->querySubObject("Cells(int, int)", row + 2, 2);
            cellB->dynamicCall("SetValue(const QVariant&)", m_proxyModel->data(m_proxyModel->index(row, courseNameCol), Qt::DisplayRole));
            cellB->querySubObject("Borders")->setProperty("LineStyle", 1);
            cellB->setProperty("HorizontalAlignment", -4108);
            delete cellB;

            // 成绩（第3列）
            QAxObject *cellC = workSheet->querySubObject("Cells(int, int)", row + 2, 3);
            cellC->dynamicCall("SetValue(const QVariant&)", m_proxyModel->data(m_proxyModel->index(row, scoreCol), Qt::DisplayRole));
            cellC->querySubObject("Borders")->setProperty("LineStyle", 1);
            cellC->setProperty("HorizontalAlignment", -4108);
            delete cellC;

            // 考试日期（第4列）
            QAxObject *cellD = workSheet->querySubObject("Cells(int, int)", row + 2, 4);
            cellD->dynamicCall("SetValue(const QVariant&)", m_proxyModel->data(m_proxyModel->index(row, examDateCol), Qt::DisplayRole));
            cellD->querySubObject("Borders")->setProperty("LineStyle", 1);
            cellD->setProperty("HorizontalAlignment", -4108);
            delete cellD;
        }

        // ========== 3. 设置列宽 ==========
        workSheet->querySubObject("Columns(int)", 1)->setProperty("ColumnWidth", 15);
        workSheet->querySubObject("Columns(int)", 2)->setProperty("ColumnWidth", 15);
        workSheet->querySubObject("Columns(int)", 3)->setProperty("ColumnWidth", 8);
        workSheet->querySubObject("Columns(int)", 4)->setProperty("ColumnWidth", 20);

        // ========== 4. 保存与退出 ==========
        workBook->dynamicCall("SaveAs(const QString&, int, QVariant, QVariant, QVariant, QVariant, int, int)",
                              filePath, 51, QVariant(), QVariant(), QVariant(), QVariant(), 1, 2);

        delete workSheet;
        workBook->dynamicCall("Close(false)");
        delete workBook;
        delete workBooks;
        excel->dynamicCall("Quit()");
        excel->clear();
        delete excel;

#ifdef Q_OS_WIN
        system("taskkill /f /im EXCEL.EXE >nul 2>&1");
#endif

        return true;
    } catch (...) {
        if (workSheet) delete workSheet;
        if (workBook) {
            workBook->dynamicCall("Close(false)");
            delete workBook;
        }
        if (workBooks) delete workBooks;
        if (excel) {
            excel->dynamicCall("Quit()");
            excel->clear();
            delete excel;
        }
#ifdef Q_OS_WIN
        system("taskkill /f /im EXCEL.EXE >nul 2>&1");
#endif

        QMessageBox::critical(this, "错误", "导出Excel过程中发生异常！");
        return false;
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

void ScoreStatWidget::on_btnExportExcel_clicked()
{
    // 获取筛选条件
    QString className = ui->cbxClass->currentText();
    QString courseName = ui->cbxCourse->currentText();

    // 生成默认文件名
    QString defaultFileName = QString("成绩统计报表_%1_%2_%3.xlsx")
                                  .arg(className == "全部" ? "所有班级" : className)
                                  .arg(courseName == "全部" ? "所有科目" : courseName)
                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    // 选择保存路径
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存Excel文件",
        QDir::homePath() + "/" + defaultFileName,
        "Excel文件 (*.xlsx);;Excel 97-2003 (*.xls);;所有文件 (*.*)"
        );

    if (filePath.isEmpty()) {
        return; // 用户取消
    }

    // 确保文件扩展名
    if (!filePath.endsWith(".xlsx", Qt::CaseInsensitive) &&
        !filePath.endsWith(".xls", Qt::CaseInsensitive)) {
        filePath += ".xlsx";
    }

    // 导出到Excel
    if (exportToExcel(filePath)) {
        QMessageBox::information(this, "成功", QString("报表已成功导出到：\n%1").arg(filePath));

        // 询问是否打开文件
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "打开文件",
            "是否现在打开Excel文件？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
            );

        if (reply == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    } else {
        QMessageBox::critical(this, "错误", "导出Excel失败！\n请确保已安装Microsoft Excel。");
    }
}
