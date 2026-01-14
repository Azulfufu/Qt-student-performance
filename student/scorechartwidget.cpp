#include "scorechartwidget.h"
#include "ui_scorechartwidget.h"

ScoreChartWidget::ScoreChartWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoreChartWidget),
    m_chart(new QChart()),
    m_series(new QLineSeries()),
    m_scatterSeries(new QScatterSeries()),
    m_chartView(nullptr),  // 初始化为空，在initChartView中创建
    m_xAxis(nullptr),
    m_yAxis(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("成绩趋势图");

    // 序列样式配置（移到initChartView前，避免重复配置）
    m_series->setName("成绩折线");
    m_series->setPen(QPen(Qt::red, 2));
    m_scatterSeries->setName("成绩点");
    m_scatterSeries->setColor(Qt::blue);
    m_scatterSeries->setMarkerSize(6);

    initChartView();
}

ScoreChartWidget::~ScoreChartWidget()
{
    // 修复：内存释放顺序（先释放子对象）
    delete m_chartView;
    delete m_xAxis;
    delete m_yAxis;
    delete m_series;
    delete m_scatterSeries;
    delete m_chart;
    delete ui;
}

// ========== 初始化图表视图（核心修改：修复布局+坐标轴管理） ==========
void ScoreChartWidget::initChartView()
{
    // 添加序列到图表
    m_chart->addSeries(m_series);
    m_chart->addSeries(m_scatterSeries);

    // X轴（日期轴）- 保存指针用于动态调整
    m_xAxis = new QDateTimeAxis();
    m_xAxis->setFormat("yyyy-MM-dd");
    m_xAxis->setTitleText("考试日期");
    m_xAxis->setLabelsColor(Qt::black);
    m_xAxis->setTickCount(5);  // 新增：控制刻度数量，避免重叠
    m_xAxis->setLabelsAngle(-45); // 新增：标签倾斜，提升可读性
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_series->attachAxis(m_xAxis);
    m_scatterSeries->attachAxis(m_xAxis);

    // Y轴（成绩轴）- 保存指针
    m_yAxis = new QValueAxis();
    m_yAxis->setRange(0, 100);
    m_yAxis->setTitleText("成绩");
    m_yAxis->setTickCount(11);
    m_yAxis->setLabelsColor(Qt::black);
    m_yAxis->setLabelFormat("%d"); // 新增：整数显示，更直观
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    m_series->attachAxis(m_yAxis);
    m_scatterSeries->attachAxis(m_yAxis);

    // 图表样式（保留你的配置，优化细节）
    QFont titleFont("微软雅黑", 14, QFont::Bold);
    m_chart->setTitleFont(titleFont);
    QPalette chartPalette = m_chart->palette();
    chartPalette.setColor(QPalette::WindowText, Qt::darkBlue);
    m_chart->setPalette(chartPalette);
    m_chart->setBackgroundBrush(Qt::white);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_chart->setAnimationOptions(QChart::SeriesAnimations); // 新增：动画效果

    // 图表视图 + 垂直布局（修复：设置最小尺寸，避免压缩）
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    m_chartView->setMinimumSize(800, 500); // 新增：固定最小尺寸

    // 布局适配（兼容UI中的widgetChartContainer）
    QVBoxLayout *chartLayout = new QVBoxLayout(ui->widgetChartContainer);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(0);
    chartLayout->addWidget(m_chartView);
    ui->widgetChartContainer->setLayout(chartLayout);
}

// ========== 加载科目列表（修改：增加空数据提示+数据库连接校验） ==========
void ScoreChartWidget::on_btnLoadCourses_clicked()
{
    ui->cbCourse->clear();

    // 新增：数据库连接校验
    if (!DBManager::getInstance().isDbOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    QString sql = "SELECT course_id, course_name FROM courses ORDER BY course_id";
    QSqlQuery query = DBManager::getInstance().execQuery(sql);

    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "错误", "查询科目失败：" + query.lastError().text());
        return;
    }

    // 新增：默认选项
    ui->cbCourse->addItem("请选择科目", "");
    while (query.next()) {
        QString itemText = QString("%1 - %2").arg(query.value(0).toString(), query.value(1).toString());
        ui->cbCourse->addItem(itemText, query.value(1).toString());
    }

    // 新增：空数据提示
    if (ui->cbCourse->count() == 1) {
        QMessageBox::information(this, "提示", "数据库中暂无科目数据！");
    }
}

// ========== 生成趋势图（核心修改：修复日期轴范围+数据有效性） ==========
void ScoreChartWidget::on_btnGenerateChart_clicked()
{
    m_series->clear();
    m_scatterSeries->clear();

    QString courseName = ui->cbCourse->currentData().toString();
    if (courseName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择科目！");
        return;
    }

    // 新增：数据库连接校验
    if (!DBManager::getInstance().isDbOpen()) {
        QMessageBox::critical(this, "错误", "数据库未连接！");
        return;
    }

    QList<QPair<QDate, qreal>> scoreData = queryScoreData(courseName);
    if (scoreData.isEmpty()) {
        m_chart->setTitle(courseName + " 成绩趋势图（无数据）");
        // 新增：重置X轴范围，避免显示1970-01-01
        m_xAxis->setRange(QDateTime::currentDateTime().addDays(-7), QDateTime::currentDateTime());
        QMessageBox::warning(this, "提示", "该科目暂无有效成绩数据！\n请先录入成绩后重试。");
        return;
    }

    // 填充数据（优化：记录日期范围）
    QDateTime minDate, maxDate;
    for (const auto& pair : scoreData) {
        QDate date = pair.first;
        qreal score = pair.second;

        // 强制转换日期（保留你的逻辑，优化有效性判断）
        QDateTime dateTime = QDateTime(date.isValid() ? date : QDate::currentDate(), QTime(0,0));
        m_series->append(dateTime.toMSecsSinceEpoch(), score);
        m_scatterSeries->append(dateTime.toMSecsSinceEpoch(), score);

        // 记录日期范围
        if (minDate.isNull() || dateTime < minDate) minDate = dateTime;
        if (maxDate.isNull() || dateTime > maxDate) maxDate = dateTime;
    }

    // 核心修复：动态调整X轴范围（避免1970-01-01）
    if (!minDate.isNull() && !maxDate.isNull()) {
        minDate = minDate.addDays(-1);  // 左边留一点余量
        maxDate = maxDate.addDays(1);   // 右边留一点余量
        m_xAxis->setRange(minDate, maxDate);
    } else {
        // 兜底：显示最近7天
        m_xAxis->setRange(QDateTime::currentDateTime().addDays(-7), QDateTime::currentDateTime());
    }

    m_chart->setTitle(courseName + " 成绩趋势图");
    // 新增：刷新图表
    m_chartView->repaint();
}

// ========== 查询成绩数据（修改：增加错误处理+成绩过滤） ==========
QList<QPair<QDate, qreal>> ScoreChartWidget::queryScoreData(const QString& courseName)
{
    QList<QPair<QDate, qreal>> dataList;

    // 精准关联3张表，兼容空日期
    QString sql = R"(
        SELECT sc.exam_date, sc.score
        FROM scores sc
        INNER JOIN courses c ON sc.course_id = c.course_id
        WHERE c.course_name = ?
        ORDER BY sc.exam_date ASC
    )";

    QSqlQuery query;
    query.prepare(sql);
    query.addBindValue(courseName);

    // 新增：执行结果校验
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询成绩失败：" + query.lastError().text());
        return dataList;
    }

    // 解析数据（兼容空日期）
    while (query.next()) {
        QString dateStr = query.value(0).toString().trimmed();
        QDate examDate = QDate::fromString(dateStr, "yyyy-MM-dd");

        // 空日期/无效日期 → 替换为当前日期
        if (!examDate.isValid() || dateStr.isEmpty()) {
            examDate = QDate::currentDate();
        }

        qreal score = query.value(1).toDouble();
        // 过滤无效成绩（0-100之外）+ 新增：非数字过滤
        if (score >= 0 && score <= 100 && !qIsNaN(score)) {
            dataList.append({examDate, score});
        }
    }

    // 新增：按日期排序，确保趋势图顺序正确
    std::sort(dataList.begin(), dataList.end(),
              [](const QPair<QDate, qreal>& a, const QPair<QDate, qreal>& b) {
                  return a.first < b.first;
              });

    return dataList;
}
