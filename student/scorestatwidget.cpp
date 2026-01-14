#include "scorestatwidget.h"
#include "ui_scorestatwidget.h"

ScoreStatWidget::ScoreStatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreStatWidget)
{
    ui->setupUi(this);
}

ScoreStatWidget::~ScoreStatWidget()
{
    delete ui;
}
