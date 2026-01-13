#include "scoreinputwidget.h"
#include "ui_scoreinputwidget.h"


ScoreInputWidget::ScoreInputWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreInputWidget)
{
    ui->setupUi(this);

    loadClasses();
    loadCourses();

    ui->tableStudents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableStudents->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->tableStudents->horizontalHeader()->setStretchLastSection(true);
}

ScoreInputWidget::~ScoreInputWidget()
{
    delete ui;
}

// 加载所有班级到下拉框（cbxClass）
void ScoreInputWidget::loadClasses()
{
    // 查询不重复的班级名称并排序
    QString sql = "SELECT DISTINCT class_name FROM students ORDER BY class_name";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    // 清空下拉框，避免重复数据
    ui->cbxClass->clear();
    // 遍历查询结果，添加到下拉框
    while (query.next()) {
        ui->cbxClass->addItem(query.value(0).toString());
    }


    if (ui->cbxClass->count() == 0) {
        QMessageBox::information(this, "提示", "数据库中暂无班级数据，请先添加学生！");
    }
}

// 加载所有课程到下拉框（cbxCourse）
void ScoreInputWidget::loadCourses()
{
    // 查询课程ID和名称，用于下拉框（显示名称，存储ID）
    QString sql = "SELECT course_id, course_name FROM courses ORDER BY course_id";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    // 清空下拉框
    ui->cbxCourse->clear();
    // 遍历结果：addItem(显示文本, 附加数据)
    while (query.next()) {
        int courseId = query.value(0).toInt();
        QString courseName = query.value(1).toString();
        ui->cbxCourse->addItem(courseName, courseId);
    }


    if (ui->cbxCourse->count() == 0) {
        QMessageBox::information(this, "提示", "数据库中暂无课程数据，请先添加课程！");
    }
}

void ScoreInputWidget::on_btnLoadStudents_clicked()
{
    // 获取选中的班级名称
    QString className = ui->cbxClass->currentText();
    if (className.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择班级！");
        return;
    }

    // 查询该班级的所有学生（ID + 姓名）
    QString sql = QString("SELECT student_id, student_name FROM students WHERE class_name = '%1' ORDER BY student_id").arg(className);
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    // 清空表格原有数据
    ui->tableStudents->setRowCount(0);
    // 设置表格列数和表头
    ui->tableStudents->setColumnCount(3);
    ui->tableStudents->setHorizontalHeaderLabels({"学生ID", "学生姓名", "成绩"});

    // 遍历查询结果，填充表格
    while (query.next()) {
        int row = ui->tableStudents->rowCount();
        ui->tableStudents->insertRow(row); // 新增一行

        // 1. 学生ID列（只读）
        QTableWidgetItem *itemId = new QTableWidgetItem(query.value(0).toString());
        itemId->setFlags(itemId->flags() & ~Qt::ItemIsEditable); // 取消编辑权限
        itemId->setTextAlignment(Qt::AlignCenter); // 居中显示
        ui->tableStudents->setItem(row, 0, itemId);

        // 2. 学生姓名列（只读）
        QTableWidgetItem *itemName = new QTableWidgetItem(query.value(1).toString());
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
        itemName->setTextAlignment(Qt::AlignCenter);
        ui->tableStudents->setItem(row, 1, itemName);

        // 3. 成绩列（可编辑，默认值0）
        QTableWidgetItem *itemScore = new QTableWidgetItem("0");
        itemScore->setTextAlignment(Qt::AlignCenter);
        ui->tableStudents->setItem(row, 2, itemScore);
    }

    if (ui->tableStudents->rowCount() == 0) {
        QMessageBox::information(this, "提示", QString("【%1】暂无学生数据！").arg(className));
    }
}
