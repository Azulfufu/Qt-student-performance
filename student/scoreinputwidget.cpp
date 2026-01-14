#include "scoreinputwidget.h"
#include "ui_scoreinputwidget.h"
#include "dbmanager.h"
#include <QMessageBox>
#include <QDate>
#include <QDebug>
#include <QTableWidgetItem>

ScoreInputWidget::ScoreInputWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoreInputWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("成绩录入");

    // 初始化批量表格列
    ui->tableBatchScore->setColumnCount(5);
    ui->tableBatchScore->setHorizontalHeaderLabels({"学生ID", "学生姓名", "科目", "成绩", "考试日期"});
    // 设置日期默认值为当前日期
    ui->dateEditExam->setDate(QDate::currentDate());
}

ScoreInputWidget::~ScoreInputWidget()
{
    delete ui;
}

// ========== 核心工具函数：通过科目名称获取course_id ==========
int ScoreInputWidget::getCourseIdByName(const QString& courseName)
{
    // 空值保护
    if (courseName.isEmpty()) return -1;

    // 查询course_id（防SQL注入：使用预处理）
    QString sql = "SELECT course_id FROM courses WHERE course_name = ?";
    QSqlQuery query;
    query.prepare(sql);
    query.addBindValue(courseName);
    query.exec();

    if (query.next()) {
        return query.value(0).toInt();
    }
    return -1; // 未找到返回-1
}

// ========== 校验成绩合法性 ==========
bool ScoreInputWidget::validateScore(const QString& scoreStr)
{
    bool ok;
    float score = scoreStr.toFloat(&ok);
    return ok && score >= 0 && score <= 100;
}

// ========== 单条成绩录入（核心修复：适配course_id） ==========
void ScoreInputWidget::on_btnSingleSubmit_clicked()
{
    // 1. 前置校验：数据库连接
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    // 2. 获取输入数据
    QString studentId = ui->cbStudent->currentData().toString();
    QString courseName = ui->leCourse->text().trimmed();
    QString scoreStr = ui->leScore->text().trimmed();
    QString examDate = ui->dateEditExam->date().toString("yyyy-MM-dd");

    // 3. 空值校验
    if (studentId.isEmpty() || courseName.isEmpty() || scoreStr.isEmpty()) {
        QMessageBox::warning(this, "提示", "学生/科目/成绩不能为空！");
        return;
    }

    // 4. 成绩格式校验
    if (!validateScore(scoreStr)) {
        QMessageBox::warning(this, "提示", "成绩需为0-100的数字！");
        return;
    }

    // 5. 关键：获取科目对应的course_id
    int courseId = getCourseIdByName(courseName);
    if (courseId == -1) {
        QMessageBox::warning(this, "提示", QString("科目【%1】不存在，请先在courses表中添加！").arg(courseName));
        return;
    }

    // 6. 重复数据校验（使用course_id）
    QString checkSql = "SELECT * FROM scores WHERE student_id = ? AND course_id = ? AND exam_date = ?";
    QSqlQuery checkQuery;
    checkQuery.prepare(checkSql);
    checkQuery.addBindValue(studentId);
    checkQuery.addBindValue(courseId);
    checkQuery.addBindValue(examDate);
    checkQuery.exec();

    if (checkQuery.next()) {
        QMessageBox::warning(this, "提示", "该学生该科目该日期的成绩已存在！");
        return;
    }

    // 7. 插入数据库（核心：使用course_id字段）
    QString insertSql = "INSERT INTO scores (student_id, course_id, score, exam_date) VALUES (?, ?, ?, ?)";
    QSqlQuery insertQuery;
    insertQuery.prepare(insertSql);
    insertQuery.addBindValue(studentId);    // student_id（数字）
    insertQuery.addBindValue(courseId);     // course_id（数字）
    insertQuery.addBindValue(scoreStr);     // score（数字）
    insertQuery.addBindValue(examDate);     // exam_date（字符串）

    // 执行插入并处理结果
    if (!insertQuery.exec()) {
        QMessageBox::critical(this, "错误", QString("成绩录入失败：%1").arg(insertQuery.lastError().text()));
        qDebug() << "插入失败SQL：" << insertSql; // 调试用
    } else {
        QMessageBox::information(this, "成功", "成绩录入完成！");
        // 清空输入框
        ui->leCourse->clear();
        ui->leScore->clear();
    }
}

// ========== 加载学生列表到下拉框（带完整校验） ==========
void ScoreInputWidget::on_btnLoadStudents_clicked()
{
    ui->cbStudent->clear();

    // 数据库连接校验
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    // 查询学生数据
    QString sql = "SELECT student_id, student_name FROM students";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    // 处理查询错误
    if (query.lastError().isValid()) {
        QMessageBox::warning(this, "错误", QString("查询学生失败：%1").arg(query.lastError().text()));
        return;
    }

    // 无学生数据提示
    if (!query.next()) {
        QMessageBox::warning(this, "提示", "students表中暂无学生数据，请先添加！");
        return;
    }

    // 重置游标到开头，填充下拉框
    query.seek(-1);
    while (query.next()) {
        QString studentName = query.value(1).toString();
        QString studentId = query.value(0).toString();
        QString itemText = QString("%1 (ID:%2)").arg(studentName, studentId);
        ui->cbStudent->addItem(itemText, studentId);
    }
}

// ========== 批量录入：加载学生到表格 ==========
void ScoreInputWidget::on_btnLoadBatchStudents_clicked()
{
    ui->tableBatchScore->clearContents();
    ui->tableBatchScore->setRowCount(0);

    // 数据库连接校验
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    // 查询学生数据
    QString sql = "SELECT student_id, student_name FROM students";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    // 无学生数据提示
    if (!query.next()) {
        QMessageBox::warning(this, "提示", "暂无学生数据！");
        return;
    }

    // 填充表格
    query.seek(-1);
    int row = 0;
    while (query.next()) {
        ui->tableBatchScore->insertRow(row);

        // 学生ID（不可编辑）
        QTableWidgetItem *idItem = new QTableWidgetItem(query.value(0).toString());
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        ui->tableBatchScore->setItem(row, 0, idItem);

        // 学生姓名（不可编辑）
        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->tableBatchScore->setItem(row, 1, nameItem);

        // 科目（空，手动填写）
        ui->tableBatchScore->setItem(row, 2, new QTableWidgetItem(""));

        // 成绩（空，手动填写）
        ui->tableBatchScore->setItem(row, 3, new QTableWidgetItem(""));

        // 日期（当前日期）
        ui->tableBatchScore->setItem(row, 4, new QTableWidgetItem(QDate::currentDate().toString("yyyy-MM-dd")));

        row++;
    }
}

// ========== 批量录入：提交批量成绩 ==========
void ScoreInputWidget::on_btnBatchSubmit_clicked()
{
    // 数据库连接校验
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    int successCount = 0;
    int failCount = 0;

    // 遍历表格行
    for (int row = 0; row < ui->tableBatchScore->rowCount(); row++) {
        // 校验单元格是否存在
        QTableWidgetItem *idItem = ui->tableBatchScore->item(row, 0);
        QTableWidgetItem *courseItem = ui->tableBatchScore->item(row, 2);
        QTableWidgetItem *scoreItem = ui->tableBatchScore->item(row, 3);
        QTableWidgetItem *dateItem = ui->tableBatchScore->item(row, 4);

        if (!idItem || !courseItem || !scoreItem || !dateItem) {
            failCount++;
            continue;
        }

        // 获取单元格数据
        QString studentId = idItem->text().trimmed();
        QString courseName = courseItem->text().trimmed();
        QString scoreStr = scoreItem->text().trimmed();
        QString examDate = dateItem->text().trimmed();

        // 基础校验
        if (courseName.isEmpty() || scoreStr.isEmpty() || !validateScore(scoreStr)) {
            failCount++;
            continue;
        }

        // 获取course_id
        int courseId = getCourseIdByName(courseName);
        if (courseId == -1) {
            failCount++;
            continue;
        }

        // 插入数据库
        QString insertSql = "INSERT INTO scores (student_id, course_id, score, exam_date) VALUES (?, ?, ?, ?)";
        QSqlQuery insertQuery;
        insertQuery.prepare(insertSql);
        insertQuery.addBindValue(studentId);
        insertQuery.addBindValue(courseId);
        insertQuery.addBindValue(scoreStr);
        insertQuery.addBindValue(examDate);

        if (insertQuery.exec()) {
            successCount++;
        } else {
            failCount++;
            qDebug() << "批量插入失败行" << row << "：" << insertQuery.lastError().text();
        }
    }

    // 显示结果
    QMessageBox::information(this, "批量录入结果",
                             QString("成功录入：%1条\n失败：%2条").arg(successCount).arg(failCount));
    // 清空表格
    ui->tableBatchScore->clearContents();
    ui->tableBatchScore->setRowCount(0);
}
