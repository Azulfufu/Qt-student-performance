#ifndef SCORESTATWIDGET_H
#define SCORESTATWIDGET_H

#include <QWidget>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

namespace Ui {
class ScoreStatWidget;
}

class ScoreStatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreStatWidget(QWidget *parent = nullptr);
    ~ScoreStatWidget() override;

private slots:
    // 班级下拉框变化
    void on_cbxClass_currentTextChanged(const QString &arg1);
    // 课程下拉框变化
    void on_cbxCourse_currentTextChanged(const QString &arg1);

private:
    // 初始化Model/View架构
    void initModel();
    // 加载筛选下拉框数据
    void loadFilterOptions();
    // 执行数据筛选
    void filterData();
    // 统计成绩（平均分/最高分/最低分）
    void statScores();
    // 新增：加载班级列表到下拉框
    void loadClassList();
    // 新增：加载科目列表到下拉框
    void loadCourseList();

    Ui::ScoreStatWidget *ui;
    QSqlRelationalTableModel *m_relModel; // 关联模型
    QSortFilterProxyModel *m_proxyModel;  // 代理模型（筛选用）
};

#endif // SCORESTATWIDGET_H
