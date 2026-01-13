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

