#include "statsdialog.h"
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLineSeries>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QDate>
#include <QPen>
#include <QScrollBar>
#include <QVector>
#include <algorithm>
#include <limits>

StatsDialog::StatsDialog(const QVector<WorkoutData>& workouts, QWidget *parent)
    : QDialog(parent), allWorkouts(workouts), currentStartDate(QDate::currentDate()), currentEndDate(QDate::currentDate())
{
    setWindowTitle("Статистика тренировок");
    resize(1000, 700);

    currentLayout = new QVBoxLayout(this);
    currentLayout->setContentsMargins(0, 0, 0, 0);

    setupUI();
}

void StatsDialog::updateData(const QVector<WorkoutData>& workouts)
{
    allWorkouts = workouts;
    setupUI();
}

void StatsDialog::setupUI() {
    QLayoutItem* child;
    while ((child = currentLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    // Создаем элементы управления
    QWidget *controlsWidget = new QWidget();
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(10, 10, 10, 10);

    sportsCombo = new QComboBox(this);
    sportsCombo->setFixedWidth(200);
    sportsCombo->setStyleSheet("QComboBox { font-size: 14px; }");

    periodCombo = new QComboBox(this);
    periodCombo->addItems({"Неделя", "Месяц", "Год"});
    periodCombo->setCurrentIndex(currentPeriod);
    periodCombo->setFixedWidth(150);
    periodCombo->setStyleSheet("QComboBox { font-size: 14px; }");

    // Кнопки навигации
    prevPeriodButton = new QPushButton("◀", this);
    nextPeriodButton = new QPushButton("▶", this);
    prevPeriodButton->setFixedSize(30, 30);
    nextPeriodButton->setFixedSize(30, 30);

    QString buttonStyle = "QPushButton {"
                         "   border: none;"
                         "   background: #f0f0f0;"
                         "   font-size: 14px;"
                         "   border-radius: 4px;"
                         "}"
                         "QPushButton:hover {"
                         "   background: #e0e0e0;"
                         "}"
                         "QPushButton:disabled {"
                         "   color: #aaa;"
                         "}";
    prevPeriodButton->setStyleSheet(buttonStyle);
    nextPeriodButton->setStyleSheet(buttonStyle);

    // Метка для отображения текущего периода
    QLabel *periodLabel = new QLabel(this);
    periodLabel->setAlignment(Qt::AlignCenter);
    periodLabel->setStyleSheet("QLabel { font-weight: bold; }");

    // Компоновка элементов управления
    controlsLayout->addWidget(prevPeriodButton);
    controlsLayout->addWidget(sportsCombo);
    controlsLayout->addStretch();
    controlsLayout->addWidget(periodCombo);
    controlsLayout->addWidget(nextPeriodButton);
    controlsLayout->addWidget(periodLabel);

    // Область с графиками
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(20);

    chartsContainer = new QWidget();
    chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setSpacing(20);

    setupCharts(allWorkouts, contentLayout);

    contentLayout->addWidget(chartsContainer);
    scrollArea->setWidget(contentWidget);

    currentLayout->addWidget(controlsWidget);
    currentLayout->addWidget(scrollArea);

    connect(sportsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatsDialog::showSportDetails);
    connect(periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StatsDialog::updateTimePeriod);
    connect(prevPeriodButton, &QPushButton::clicked, this, [this]() { shiftPeriod(-1); });
    connect(nextPeriodButton, &QPushButton::clicked, this, [this]() { shiftPeriod(1); });

    // Обновляем кнопки навигации
    updateNavigationButtons();
}

void StatsDialog::shiftPeriod(int direction)
{
    currentShift += direction;
    showSportDetails(sportsCombo->currentIndex());
    updateNavigationButtons();
}

void StatsDialog::updateNavigationButtons()
{
    prevPeriodButton->setEnabled(true);
    nextPeriodButton->setEnabled(true);
}

void StatsDialog::setupCharts(const QVector<WorkoutData>& workouts, QVBoxLayout *layout)
{
    QLayoutItem* child;
    while ((child = chartsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    sportsCombo->clear();

    if (workouts.isEmpty()) {
        QLabel *noDataLabel = new QLabel("Нет данных для отображения статистики");
        noDataLabel->setAlignment(Qt::AlignCenter);
        chartsLayout->addWidget(noDataLabel);
        return;
    }

    // Собираем уникальные виды спорта
    QSet<QString> uniqueSports;
    for (const WorkoutData &workout : workouts) {
        if (!workout.type.isEmpty()) {
            uniqueSports.insert(workout.type);
        }
    }

    if (uniqueSports.isEmpty()) {
        QLabel *noValidDataLabel = new QLabel("Нет валидных данных для статистики");
        noValidDataLabel->setAlignment(Qt::AlignCenter);
        chartsLayout->addWidget(noValidDataLabel);
        return;
    }

    // Заполняем комбобокс видами спорта
    sportsCombo->addItems(uniqueSports.values());

    // Заголовок
    QLabel *titleLabel = new QLabel("<h2>Статистика тренировок</h2>");
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setAlignment(Qt::AlignCenter);
    chartsLayout->addWidget(titleLabel);

    if (!uniqueSports.isEmpty()) {
        showSportDetails(0);
    }
}

void StatsDialog::updateTimePeriod(int index)
{
    currentPeriod = index;
    currentShift = 0;
    showSportDetails(sportsCombo->currentIndex());
}

void StatsDialog::showSportDetails(int index)
{
    if (index < 0 || index >= sportsCombo->count()) return;

    QString sportName = sportsCombo->itemText(index);

    // Очищаем предыдущие графики
    QLayoutItem* child;
    while ((child = chartsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // Фильтруем тренировки по виду спорта
    QVector<WorkoutData> sportWorkouts;
    for (const WorkoutData &workout : allWorkouts) {
        if (workout.type == sportName) {
            sportWorkouts.append(workout);
        }
    }

    if (sportWorkouts.isEmpty()) {
        QLabel *noDataLabel = new QLabel("Нет данных для отображения");
        noDataLabel->setAlignment(Qt::AlignCenter);
        chartsLayout->addWidget(noDataLabel);
        return;
    }

    // Определяем границы периода с учетом сдвига
    QDate currentDate = QDate::currentDate();
    QDate startDate, endDate;

    switch(currentPeriod) {
        case 0: {
            startDate = currentDate.addDays(7 * currentShift).addDays(-(currentDate.dayOfWeek() - 1));
            endDate = startDate.addDays(6);
            break;
        }
        case 1: {
            startDate = currentDate.addMonths(currentShift);
            startDate = QDate(startDate.year(), startDate.month(), 1);
            endDate = startDate.addMonths(1).addDays(-1);
            break;
        }
        case 2: {
            startDate = currentDate.addYears(currentShift);
            startDate = QDate(startDate.year(), 1, 1);
            endDate = QDate(startDate.year(), 12, 31);
            break;
        }
    }

    currentStartDate = startDate;
    currentEndDate = endDate;

    QVector<WorkoutData> periodWorkouts;
    for (const WorkoutData &workout : sportWorkouts) {
        if (workout.date >= startDate && workout.date <= endDate) {
            periodWorkouts.append(workout);
        }
    }

    // Сортируем по дате
    std::sort(periodWorkouts.begin(), periodWorkouts.end(), [](const WorkoutData &a, const WorkoutData &b) {
        return a.date < b.date;
    });

    // Группируем данные
    QMap<QString, QVector<double>> durationData, caloriesData;
    QStringList categories;

    switch(currentPeriod) {
        case 0: {
            for (const WorkoutData &w : periodWorkouts) {
                QString dayStr = w.date.toString("dd.MM");
                if (!categories.contains(dayStr)) {
                    categories.append(dayStr);
                }
                durationData[dayStr].append(w.duration);
                caloriesData[dayStr].append(w.calories);
            }
            break;
        }
        case 1: {
            for (const WorkoutData &w : periodWorkouts) {
                QDate weekStart = w.date.addDays(-(w.date.dayOfWeek() - 1));
                QDate weekEnd = weekStart.addDays(6);
                QString weekStr = QString("%1-%2").arg(
                    weekStart.toString("dd.MM"),
                    weekEnd.toString("dd.MM")
                );
                if (!categories.contains(weekStr)) {
                    categories.append(weekStr);
                }
                durationData[weekStr].append(w.duration);
                caloriesData[weekStr].append(w.calories);
            }
            break;
        }
        case 2: {
            for (const WorkoutData &w : periodWorkouts) {
                QString monthStr = QLocale().monthName(w.date.month(), QLocale::ShortFormat);
                if (!categories.contains(monthStr)) {
                    categories.append(monthStr);
                }
                durationData[monthStr].append(w.duration);
                caloriesData[monthStr].append(w.calories);
            }
            break;
        }
    }

    QVector<double> durations, calories, intensities;
    for (const QString &category : categories) {
        double totalDuration = 0, totalCalories = 0;
        int count = durationData[category].size();

        for (double val : durationData[category]) {
            totalDuration += val;
        }
        for (double val : caloriesData[category]) {
            totalCalories += val;
        }

        if (currentPeriod == 0) {
            durations.append(qRound(totalDuration * 10) / 10.0);
            calories.append(qRound(totalCalories * 10) / 10.0);
        } else {
            durations.append(count > 0 ? qRound((totalDuration / count) * 10) / 10.0 : 0);
            calories.append(count > 0 ? qRound((totalCalories / count) * 10) / 10.0 : 0);
        }

        // Интенсивность (ккал/мин)
        double intensity = 0;
        if (totalDuration > 0) {
            intensity = totalCalories / totalDuration;
            intensity = qRound(intensity * 10) / 10.0;
        }
        intensities.append(intensity);
    }

    // Добавляем метку с периодом
    QString periodLabelText;
    switch(currentPeriod) {
        case 0: // Неделя
            periodLabelText = QString("Неделя: %1 - %2")
                .arg(startDate.toString("dd.MM.yyyy"))
                .arg(endDate.toString("dd.MM.yyyy"));
            break;
        case 1: // Месяц
            periodLabelText = QString("Месяц: %1")
                .arg(QLocale().monthName(startDate.month()) + " " + QString::number(startDate.year()));
            break;
        case 2: // Год
            periodLabelText = QString("Год: %1").arg(startDate.year());
            break;
    }

    QLabel *periodInfoLabel = new QLabel(periodLabelText);
    periodInfoLabel->setAlignment(Qt::AlignCenter);
    periodInfoLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin-bottom: 10px; }");
    chartsLayout->addWidget(periodInfoLabel);

    // Создаем графики
    if (!durations.isEmpty()) {
        QString title = currentPeriod == 0 ? "Длительность тренировок (" + sportName + ")"
                      : "Средняя длительность (" + sportName + ")";
        createScrollableChart(title, "Минуты", categories, durations, "мин");
    }
    if (!calories.isEmpty()) {
        QString title = currentPeriod == 0 ? "Сожженные калории (" + sportName + ")"
                      : "Средние калории (" + sportName + ")";
        createScrollableChart(title, "Ккал", categories, calories, "ккал");
    }
    if (!intensities.isEmpty()) {
        createScrollableChart("Интенсивность (" + sportName + ")", "Ккал/мин", categories, intensities, "ккал/мин");
    }

    updateNavigationButtons();
}

void StatsDialog::createScrollableChart(const QString &title, const QString &yTitle,
                                      const QStringList &categories, const QVector<double> &values,
                                      const QString &unit)
{
    if (categories.isEmpty() || values.isEmpty()) return;

    QWidget *chartContainer = new QWidget();
    QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(5);

    // Заголовок
    QLabel *titleLabel = new QLabel(QString("<h3>%1</h3>").arg(title));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { margin-bottom: 5px; }");
    containerLayout->addWidget(titleLabel);

    // Область прокрутки
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setMinimumHeight(300);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    QWidget *scrollContent = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(scrollContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Создаем график
    QChart *chart = new QChart();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setMargins(QMargins(5, 5, 5, 5));
    chart->legend()->hide();
    chart->setBackgroundRoundness(0);
    chart->setBackgroundBrush(Qt::white);

    // Линия данных
    QLineSeries *series = new QLineSeries();
    series->setName(yTitle);

    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::min();

    for (int i = 0; i < values.size(); ++i) {
        series->append(i, values[i]);
        if (values[i] < minVal) minVal = values[i];
        if (values[i] > maxVal) maxVal = values[i];
    }

    // Стиль линии
    QPen pen(QColor("#4285F4"), 2);
    if (title.contains("калории", Qt::CaseInsensitive)) {
        pen.setColor(QColor("#34A853"));
    } else if (title.contains("Интенсивность")) {
        pen.setColor(QColor("#EA4335"));
    }
    series->setPen(pen);
    series->setPointsVisible(true);

    // Настройка точек
    series->setPointLabelsFormat("@yPoint " + unit);
    series->setPointLabelsVisible(true);
    series->setPointLabelsFont(QFont("Arial", 8));
    series->setPointLabelsColor(pen.color().darker());

    chart->addSeries(series);

    // Ось X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Период");
    axisX->setLabelsAngle(currentPeriod == 0 ? -45 : 0);
    axisX->setLabelsFont(QFont("Arial", 8));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Ось Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(yTitle);
    axisY->setLabelFormat("%.1f");
    axisY->setLabelsFont(QFont("Arial", 8));

    double range = maxVal - minVal;
    if (range < 0.1) {
        axisY->setRange(0, maxVal * 1.5);
    } else {
        axisY->setRange(qMax(0.0, minVal - range * 0.1), maxVal + range * 0.1);
    }
    axisY->applyNiceNumbers();

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Отображение графика
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumWidth(qMax(600, categories.size() * 80));
    chartView->setStyleSheet("background: white;");
    chartView->setRubberBand(QChartView::NoRubberBand);
    chartView->setInteractive(false);

    contentLayout->addWidget(chartView);
    scrollContent->setLayout(contentLayout);
    scrollArea->setWidget(scrollContent);

    containerLayout->addWidget(scrollArea);
    chartsLayout->addWidget(chartContainer);
}
