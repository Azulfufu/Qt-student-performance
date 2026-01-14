#include "scorechartwidget.h"
#include "ui_ScoreChartWidget.h"

ScoreChartWidget::ScoreChartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreChartWidget)
{
    ui->setupUi(this);
}

ScoreChartWidget::~ScoreChartWidget()
{
    delete ui;
}
