#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include "mainwindow.h"

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QComboBox>

QT_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QBarSeries;
class QBarSet;
class QValueAxis;
class QLineSeries;
QT_END_NAMESPACE

class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsDialog(const QVector<WorkoutData>& workouts, QWidget *parent = nullptr);
    void updateData(const QVector<WorkoutData>& workouts);

private slots:
    void showSportDetails(int index);
    void updateTimePeriod(int index);
    void setupUI();
    void shiftPeriod(int direction);
    void updateNavigationButtons();

private:
    int currentShift = 0;
    QPushButton *prevPeriodButton;
    QPushButton *nextPeriodButton;
    QString getWeekRangeString(const QDate &date) const;
    void setupCharts(const QVector<WorkoutData>& workouts, QVBoxLayout *layout);
    QVector<WorkoutData> filterWorkoutsByPeriod(const QVector<WorkoutData>& workouts);
    void createScrollableChart(const QString &title, const QString &yTitle,
                             const QStringList &dates, const QVector<double> &values,
                             const QString &unit);

    QScrollArea *chartsScrollArea;
    QWidget *scrollContent;
    QHBoxLayout *scrollLayout;
    QList<QChartView*> chartViews;
    int currentChartIndex = 0;

    void updateChartsNavigation();
    void showChart(int index);

    QVBoxLayout *currentLayout;
    QVector<WorkoutData> allWorkouts;
    QComboBox *sportsCombo;
    QComboBox *periodCombo;
    QWidget *chartsContainer;
    QVBoxLayout *chartsLayout;
    int currentPeriod = 0;
    QDate currentStartDate;
    QDate currentEndDate;
};

#endif // STATSDIALOG_H
