#include "scoreinputwidget.h"
#include "ui_scoreinputwidget.h" // 由uic工具生成，关联UI文件

// 构造函数：初始化UI + 加载基础数据
ScoreInputWidget::ScoreInputWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreInputWidget) // 创建UI对象
{
    ui->setupUi(this); // 初始化UI控件

    // 初始化时加载班级和课程列表
    loadClasses();
    loadCourses();

    ui->tableStudents->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    ui->tableStudents->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed); // 双击编辑成绩
    ui->tableStudents->horizontalHeader()->setStretchLastSection(true); // 最后一列自适应宽度
}

