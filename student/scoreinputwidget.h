#ifndef SCOREINPUTWIDGET_H
#define SCOREINPUTWIDGET_H

#include <QWidget>
#include <QSqlQuery>
#include <QDate>

namespace Ui {
class ScoreInputWidget;
}

class ScoreInputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreInputWidget(QWidget *parent = nullptr);
    ~ScoreInputWidget() override;

private slots:
    // 单条成绩录入提交
    void on_btnSingleSubmit_clicked();
    // 加载学生列表到下拉框
    void on_btnLoadStudents_clicked();
    // 加载批量学生到表格
    void on_btnLoadBatchStudents_clicked();
    // 批量提交成绩
    void on_btnBatchSubmit_clicked();

private:
    // 工具函数：通过科目名称获取course_id
    int getCourseIdByName(const QString& courseName);
    // 工具函数：校验成绩合法性（0-100的数字）
    bool validateScore(const QString& scoreStr);

    Ui::ScoreInputWidget *ui;
};

#endif // SCOREINPUTWIDGET_H
