#ifndef SCOREINPUTWIDGET_H
#define SCOREINPUTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDate>
#include "dbmanager.h"

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

    void on_btnLoadStudents_clicked();
    void on_btnSaveScores_clicked();

private:

    void loadClasses();
    void loadCourses();

    Ui::ScoreInputWidget *ui;
};

#endif // SCOREINPUTWIDGET_H
