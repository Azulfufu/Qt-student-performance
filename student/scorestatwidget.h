#ifndef SCORESTATWIDGET_H
#define SCORESTATWIDGET_H

#include <QWidget>

namespace Ui {
class ScoreStatWidget;
}

class ScoreStatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreStatWidget(QWidget *parent = nullptr);
    ~ScoreStatWidget();

private:
    Ui::ScoreStatWidget *ui;
};

#endif // SCORESTATWIDGET_H
