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
#include <QPainter>   // 新增：抗锯齿需要
#include <QFont>      // 新增：字体配置需要
#include <QPen>       // 新增：画笔配置需要
#include <algorithm>  // 新增：日期排序需要
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

private:
    void initChartView();
    QList<QPair<QDate, qreal>> queryScoreData(const QString& courseName);

    Ui::ScoreChartWidget *ui;
    QChart *m_chart;
    QLineSeries *m_series;
    QScatterSeries *m_scatterSeries;
    QChartView *m_chartView;
    // 新增：保存坐标轴指针（用于动态调整范围）
    QDateTimeAxis *m_xAxis;
    QValueAxis *m_yAxis;
};

#endif // SCORECHARTWIDGET_H
