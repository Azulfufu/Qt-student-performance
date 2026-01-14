#ifndef SCORECHARTWIDGET_H
#define SCORECHARTWIDGET_H

#include <QWidget>

namespace Ui {
class ScoreChartWidget;
}

class ScoreChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreChartWidget(QWidget *parent = nullptr);
    ~ScoreChartWidget();

private:
    Ui::ScoreChartWidget *ui;
};

#endif // SCORECHARTWIDGET_H
