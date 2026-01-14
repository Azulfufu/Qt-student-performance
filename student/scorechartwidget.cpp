#include "scorechartwidget.h"
#include "ui_scorechartwidget.h"
#include <QDebug>
#include <QSqlError>

ScoreChartWidget::ScoreChartWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoreChartWidget),
    m_chart(new QChart()),
    m_series(new QLineSeries()),
    m_scatterSeries(new QScatterSeries()),
    m_chartView(nullptr),
    m_xAxis(nullptr),
    m_yAxis(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("成绩趋势图");

    // 序列样式配置
    m_series->setName("成绩折线");
    m_series->setPen(QPen(Qt::red, 2));
    m_scatterSeries->setName("成绩点");
    m_scatterSeries->setColor(Qt::blue);
    m_scatterSeries->setMarkerSize(6);

    initChartView();
}

ScoreChartWidget::~ScoreChartWidget()
{
    delete m_chartView;
    delete m_xAxis;
    delete m_yAxis;
    delete m_series;
    delete m_scatterSeries;
    delete m_chart;
    delete ui;
}

void ScoreChartWidget::initChartView()
{
    // 添加序列到图表
    m_chart->addSeries(m_series);
    m_chart->addSeries(m_scatterSeries);

    // X轴（日期轴）
    m_xAxis = new QDateTimeAxis();
    m_xAxis->setFormat("yyyy-MM-dd");
    m_xAxis->setTitleText("考试日期");
    m_xAxis->setLabelsColor(Qt::black);
    m_xAxis->setTickCount(5);
    m_xAxis->setLabelsAngle(-45);
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_series->attachAxis(m_xAxis);
    m_scatterSeries->attachAxis(m_xAxis);

    // Y轴（成绩轴）
    m_yAxis = new QValueAxis();
    m_yAxis->setRange(0, 100);
    m_yAxis->setTitleText("成绩");
    m_yAxis->setTickCount(11);
    m_yAxis->setLabelsColor(Qt::black);
    m_yAxis->setLabelFormat("%d");
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    m_series->attachAxis(m_yAxis);
    m_scatterSeries->attachAxis(m_yAxis);

    // 图表样式
    QFont titleFont("微软雅黑", 14, QFont::Bold);
    m_chart->setTitleFont(titleFont);
    QPalette chartPalette = m_chart->palette();
    chartPalette.setColor(QPalette::WindowText, Qt::darkBlue);
    m_chart->setPalette(chartPalette);
    m_chart->setBackgroundBrush(Qt::white);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    // 图表视图 + 布局
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumSize(800, 500);

    QVBoxLayout *chartLayout = new QVBoxLayout(ui->widgetChartContainer);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(0);
    chartLayout->addWidget(m_chartView);
    ui->widgetChartContainer->setLayout(chartLayout);
}

// ========== 新增：加载学生列表到下拉框 ==========
void ScoreChartWidget::on_btnLoadStudents_clicked()
{
    ui->cbStudent->clear();

    // 数据库连接校验
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    // 查询学生数据
    QString sql = "SELECT student_id, student_name FROM students ORDER BY student_id";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "错误", "查询学生失败：" + query.lastError().text());
        return;
    }

    // 添加默认选项
    ui->cbStudent->addItem("请选择学生", "");
    while (query.next()) {
        QString itemText = QString("%1 - %2").arg(query.value(0).toString(), query.value(1).toString());
        ui->cbStudent->addItem(itemText, query.value(0).toString());
    }

    // 空数据提示
    if (ui->cbStudent->count() == 1) {
        QMessageBox::information(this, "提示", "数据库中暂无学生数据！");
    }
}

// ========== 原有：加载科目列表 ==========
void ScoreChartWidget::on_btnLoadCourses_clicked()
{
    ui->cbCourse->clear();

    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    QString sql = "SELECT course_id, course_name FROM courses ORDER BY course_id";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "错误", "查询科目失败：" + query.lastError().text());
        return;
    }

    ui->cbCourse->addItem("请选择科目", "");
    while (query.next()) {
        QString itemText = QString("%1 - %2").arg(query.value(0).toString(), query.value(1).toString());
        ui->cbCourse->addItem(itemText, query.value(1).toString());
    }

    if (ui->cbCourse->count() == 1) {
        QMessageBox::information(this, "提示", "数据库中暂无科目数据！");
    }
}

// ========== 重构：生成趋势图（支持选择学生） ==========
void ScoreChartWidget::on_btnGenerateChart_clicked()
{
    m_series->clear();
    m_scatterSeries->clear();

    // 获取选中的学生和科目
    QString studentId = ui->cbStudent->currentData().toString();
    QString courseName = ui->cbCourse->currentData().toString();

    // 校验选择
    if (studentId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择学生！");
        return;
    }
    if (courseName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择科目！");
        return;
    }

    // 数据库连接校验
    if (!DBManager::getInstance().m_db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    // 查询该学生该科目的成绩数据
    QList<QPair<QDate, qreal>> scoreData = queryScoreData(studentId, courseName);
    if (scoreData.isEmpty()) {
        QString studentName = getStudentNameById(studentId);
        m_chart->setTitle(QString("%1 - %2 成绩趋势图（无数据）").arg(studentName, courseName));
        m_xAxis->setRange(QDateTime::currentDateTime().addDays(-7), QDateTime::currentDateTime());
        QMessageBox::warning(this, "提示", QString("【%1】的【%2】科目暂无有效成绩数据！").arg(studentName, courseName));
        return;
    }

    // 填充数据并记录日期范围
    QDateTime minDate, maxDate;
    for (const auto& pair : scoreData) {
        QDate date = pair.first;
        qreal score = pair.second;

        QDateTime dateTime = QDateTime(date.isValid() ? date : QDate::currentDate(), QTime(0,0));
        m_series->append(dateTime.toMSecsSinceEpoch(), score);
        m_scatterSeries->append(dateTime.toMSecsSinceEpoch(), score);

        if (minDate.isNull() || dateTime < minDate) minDate = dateTime;
        if (maxDate.isNull() || dateTime > maxDate) maxDate = dateTime;
    }

    // 调整X轴范围
    if (!minDate.isNull() && !maxDate.isNull()) {
        minDate = minDate.addDays(-1);
        maxDate = maxDate.addDays(1);
        m_xAxis->setRange(minDate, maxDate);
    } else {
        m_xAxis->setRange(QDateTime::currentDateTime().addDays(-7), QDateTime::currentDateTime());
    }

    // 更新图表标题（显示学生姓名+科目）
    QString studentName = getStudentNameById(studentId);
    m_chart->setTitle(QString("%1 - %2 成绩趋势图").arg(studentName, courseName));
    m_chartView->repaint();

    QMessageBox::information(this, "成功", QString("已生成【%1】的【%2】科目成绩趋势图！").arg(studentName, courseName));
}

// ========== 重构：按学生+科目查询成绩数据 ==========
QList<QPair<QDate, qreal>> ScoreChartWidget::queryScoreData(const QString& studentId, const QString& courseName)
{
    QList<QPair<QDate, qreal>> dataList;

    // 获取科目ID
    int courseId = -1;
    QString getCourseIdSql = "SELECT course_id FROM courses WHERE course_name = ?";
    QSqlQuery courseQuery;
    courseQuery.prepare(getCourseIdSql);
    courseQuery.addBindValue(courseName);
    if (courseQuery.exec() && courseQuery.next()) {
        courseId = courseQuery.value(0).toInt();
    } else {
        QMessageBox::warning(this, "提示", QString("科目【%1】不存在！").arg(courseName));
        return dataList;
    }

    // 查询该学生该科目的成绩
    QString sql = R"(
        SELECT sc.exam_date, sc.score
        FROM scores sc
        WHERE sc.student_id = ? AND sc.course_id = ?
        AND sc.score >= 0 AND sc.score <= 100
        ORDER BY sc.exam_date ASC
    )";

    QSqlQuery query;
    query.prepare(sql);
    query.addBindValue(studentId);
    query.addBindValue(courseId);
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询成绩失败：" + query.lastError().text());
        return dataList;
    }

    // 解析数据
    while (query.next()) {
        QString dateStr = query.value(0).toString().trimmed();
        QDate examDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (!examDate.isValid() || dateStr.isEmpty()) {
            examDate = QDate::currentDate();
        }
        qreal score = query.value(1).toDouble();
        dataList.append({examDate, score});
    }

    // 按日期排序
    std::sort(dataList.begin(), dataList.end(),
              [](const QPair<QDate, qreal>& a, const QPair<QDate, qreal>& b) {
                  return a.first < b.first;
              });

    return dataList;
}

// ========== 新增：通过学生ID获取姓名 ==========
QString ScoreChartWidget::getStudentNameById(const QString& studentId)
{
    QString sql = "SELECT student_name FROM students WHERE student_id = ?";
    QSqlQuery query;
    query.prepare(sql);
    query.addBindValue(studentId);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "未知学生";
}
