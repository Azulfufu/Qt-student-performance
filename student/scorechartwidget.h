#ifndef SCORECHARTWIDGET_H
#define SCORECHARTWIDGET_H

#include <QWidget>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QChartView>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDate>
#include <QPalette>
#include <QVBoxLayout>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <algorithm>
#include "dbmanager.h"


    namespace Ui {
    class ScoreChartWidget;
}

class ScoreChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreChartWidget(QWidget *parent = nullptr);
    ~ScoreChartWidget() override;

private slots:
    void on_btnLoadCourses_clicked();
    void on_btnGenerateChart_clicked();
    // 新增：加载学生列表到下拉框
    void on_btnLoadStudents_clicked();

private:
    void initChartView();
    // 重构：支持按学生+科目查询成绩数据
    QList<QPair<QDate, qreal>> queryScoreData(const QString& studentId, const QString& courseName);
    // 新增：获取学生姓名（用于图表标题）
    QString getStudentNameById(const QString& studentId);

    Ui::ScoreChartWidget *ui;
    QChart *m_chart;
    QLineSeries *m_series;
    QScatterSeries *m_scatterSeries;
    QChartView *m_chartView;
    QDateTimeAxis *m_xAxis;
    QValueAxis *m_yAxis;
};

#endif // SCORECHARTWIDGET_H
