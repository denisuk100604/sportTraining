#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include <QDate>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVector>
#include <QGroupBox>
#include <QFormLayout>
#include <QMenu>
#include <QAction>
#include <QStackedWidget>
#include <QTableWidget>
#include <QCalendarWidget>
#include <QDialogButtonBox>

class Database;

struct WorkoutData {
    int id = -1;
    QString type;
    int duration;
    int sets;
    int reps;
    int calories;
    QString notes;
    QDate date;
};

class StatsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showCalendarDialog();
    void updateDays();
    void daySelected(QListWidgetItem* item);
    void prevWeek();
    void nextWeek();
    void addWorkout();
    void toggleWorkoutDetails();
    void showWorkoutContextMenu(const QPoint &pos);
    void deleteWorkout();
    void editWorkout();
    void showStats();
    void showWorkoutsPage();
    void showStatsPage();
    void addTableRow(QTableWidget *table, int row, const QString &label, const QString &value);

private:
    QLabel *currentDateLabel;
    Database *database;
    void setupUI();
    void setupCalendar();
    void updateWorkoutsDisplay();
    int findWorkoutIndex(QGroupBox* workoutBox);

    QDate m_currentDate;
    QListWidget* m_daysList;
    QComboBox* m_monthCombo;
    QComboBox* m_yearCombo;
    QPushButton *addButton;
    QWidget *workoutsContainer;
    QVBoxLayout *workoutsLayout;
    QVector<WorkoutData> workouts;
    QGroupBox* contextMenuWorkout;
    QPushButton *statsButton;
    QStackedWidget *stackedWidget;
    QWidget *workoutsPage;
    QWidget *statsPage;
    QPushButton *workoutsButton;
    QPushButton *statsPageButton;
    StatsDialog *statsDialog;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif
