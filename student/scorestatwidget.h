#ifndef SCORESTATWIDGET_H
#define SCORESTATWIDGET_H

#include <QWidget>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QMessageBox>
#include "dbmanager.h"

// 前置声明UI类（与UI文件名 ScoreStatWidget.ui 一致）
namespace Ui {
class ScoreStatWidget;
}

class ScoreStatWidget : public QWidget
{
    Q_OBJECT

public:
    // 构造/析构仅声明
    explicit ScoreStatWidget(QWidget *parent = nullptr);
    ~ScoreStatWidget() override;

private slots:
    // 筛选条件变化时触发的槽函数（与下拉框currentTextChanged关联）
    void on_cbxClass_currentTextChanged(const QString &arg1);
    void on_cbxCourse_currentTextChanged(const QString &arg1);

private:
    // 核心功能函数声明
    void initModel();          // 初始化Model/View
    void loadFilterOptions();  // 加载筛选下拉框数据（班级/课程）
    void filterData();         // 执行数据筛选
    void statScores();         // 统计平均分/最高分/最低分

    // 成员变量声明
    Ui::ScoreStatWidget *ui;
    QSqlRelationalTableModel *m_relModel;    // 关联模型（关联多表）
    QSortFilterProxyModel *m_proxyModel;     // 代理模型（用于筛选）
};

#endif // SCORESTATWIDGET_H
