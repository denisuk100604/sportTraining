#include "mainwindow.h"
#include "database.h"
#include "workoutdialog.h"
#include "statsdialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLocale>
#include <QMessageBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QFormLayout>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QTableWidget>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentDate(QDate::currentDate())
{
    setWindowTitle("Трекер тренировок");
    resize(600, 500);

    // Инициализация базы данных
    database = new Database(this);
    if (!database->openDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть базу данных");
    }

    // Загрузка данных из базы
    workouts = database->getAllWorkouts();

    // Проверка файла БД
        QFile dbFile("workout_tracker.db");
        if (!dbFile.exists()) {
            qDebug() << "Database file doesn't exist. It will be created.";
        } else {
            qDebug() << "Database file exists. Size:" << dbFile.size() << "bytes";
        }

        database = new Database(this);
        if (!database->openDatabase()) {
            QMessageBox::critical(this, "Ошибка",
                QString("Не удалось открыть базу данных.\nОшибка: %1")
                .arg(database->lastError()));
        }

    setupUI();
    updateWorkoutsDisplay();

}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (event->size().width() != event->oldSize().width()) {
        updateDays();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Создаем stacked widget для переключения страниц
    stackedWidget = new QStackedWidget(this);

    // 1. Страница тренировок
    workoutsPage = new QWidget();
    QVBoxLayout *workoutsPageLayout = new QVBoxLayout(workoutsPage);
    workoutsPageLayout->setContentsMargins(0, 0, 0, 0);
    workoutsPageLayout->setSpacing(10);

    // Панель управления датой с иконкой календаря
    QHBoxLayout *dateControlsLayout = new QHBoxLayout();
    dateControlsLayout->setContentsMargins(0, 0, 0, 0);

    // Кнопка с иконкой календаря
    QPushButton *calendarButton = new QPushButton(this);
    calendarButton->setIcon(QIcon(":/icons/calendar.png"));
    calendarButton->setIconSize(QSize(24, 24));
    calendarButton->setFlat(true);
    calendarButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   padding: 5px;"
        "   background: transparent;"
        "}"
        "QPushButton:hover {"
        "   background: #e0e0e0;"
        "   border-radius: 4px;"
        "}"
    );

    // Метка для отображения текущего месяца и года
    currentDateLabel = new QLabel(this);
    currentDateLabel->setText(QLocale(QLocale::Russian).monthName(m_currentDate.month()) +
                         " " + QString::number(m_currentDate.year()));
    currentDateLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   padding: 5px;"
        "}"
    );

    dateControlsLayout->addWidget(calendarButton);
    dateControlsLayout->addWidget(currentDateLabel);
    dateControlsLayout->addStretch();

    workoutsPageLayout->addLayout(dateControlsLayout);

    // Календарь недели с кнопками навигации
    QHBoxLayout *weekNavLayout = new QHBoxLayout();
    weekNavLayout->setContentsMargins(0, 0, 0, 0);
    weekNavLayout->setSpacing(0);

    QPushButton *prevBtn = new QPushButton("◀", this);
    prevBtn->setFixedSize(30, 40);
    prevBtn->setStyleSheet(
        "QPushButton {"
        "   font-size: 14px;"
        "   border: none;"
        "   background: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "}"
        "QPushButton:hover {"
        "   background: #e0e0e0;"
        "}"
    );

    setupCalendar();
    m_daysList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *nextBtn = new QPushButton("▶", this);
    nextBtn->setFixedSize(30, 40);
    nextBtn->setStyleSheet(
        "QPushButton {"
        "   font-size: 14px;"
        "   border: none;"
        "   background: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "}"
        "QPushButton:hover {"
        "   background: #e0e0e0;"
        "}"
    );

    weekNavLayout->addWidget(prevBtn);
    weekNavLayout->addWidget(m_daysList, 1);
    weekNavLayout->addWidget(nextBtn);

    workoutsPageLayout->addLayout(weekNavLayout);

    // Контейнер для тренировок
    workoutsContainer = new QWidget();
    workoutsLayout = new QVBoxLayout(workoutsContainer);
    workoutsLayout->setAlignment(Qt::AlignTop);
    workoutsLayout->setSpacing(10);
    workoutsLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(workoutsContainer);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border: none;"
        "   background: transparent;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "   background: transparent;"
        "}"
    );
    workoutsPageLayout->addWidget(scrollArea);

    // Кнопка добавления тренировки
    addButton = new QPushButton("Добавить тренировку", this);
    addButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );
    workoutsPageLayout->addWidget(addButton);

    // 2. Страница статистики
    statsPage = new QWidget();
    QVBoxLayout *statsPageLayout = new QVBoxLayout(statsPage);
    statsPageLayout->setContentsMargins(0, 0, 0, 0);
    statsDialog = new StatsDialog(QVector<WorkoutData>(), this);
    statsPageLayout->addWidget(statsDialog);

    // Добавляем страницы
    stackedWidget->addWidget(workoutsPage);
    stackedWidget->addWidget(statsPage);

    // Кнопки навигации между страницами
    QHBoxLayout *pageNavLayout = new QHBoxLayout();
    pageNavLayout->setContentsMargins(0, 10, 0, 0);
    pageNavLayout->setSpacing(0);

    workoutsButton = new QPushButton("Тренировки", this);
    statsPageButton = new QPushButton("Статистика", this);
    workoutsButton->setEnabled(false);

    // Стиль для кнопок навигации
    QString navButtonStyle =
        "QPushButton {"
        "   padding: 8px 16px;"
        "   border: none;"
        "   background-color: #f0f0f0;"
        "   font-size: 14px;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #ddd;"
        "   font-weight: bold;"
        "   border-top: 2px solid #4CAF50;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}";

    workoutsButton->setStyleSheet(navButtonStyle);
    statsPageButton->setStyleSheet(navButtonStyle);

    pageNavLayout->addWidget(workoutsButton);
    pageNavLayout->addWidget(statsPageButton);

    mainLayout->addWidget(stackedWidget);
    mainLayout->addLayout(pageNavLayout);


    connect(addButton, &QPushButton::clicked, this, &MainWindow::addWorkout);
    connect(workoutsButton, &QPushButton::clicked, this, &MainWindow::showWorkoutsPage);
    connect(statsPageButton, &QPushButton::clicked, this, &MainWindow::showStatsPage);
    connect(prevBtn, &QPushButton::clicked, this, &MainWindow::prevWeek);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::nextWeek);
    connect(calendarButton, &QPushButton::clicked, this, &MainWindow::showCalendarDialog);

    setCentralWidget(centralWidget);
    updateDays();
}


void MainWindow::showWorkoutsPage()
{
    stackedWidget->setCurrentWidget(workoutsPage);
    workoutsButton->setEnabled(false);
    statsPageButton->setEnabled(true);
}

void MainWindow::showStatsPage() {
    if (!statsDialog) {
        statsDialog = new StatsDialog(workouts, this);
    } else {
        statsDialog->updateData(workouts);
    }
    stackedWidget->setCurrentWidget(statsPage);
    workoutsButton->setEnabled(true);
    statsPageButton->setEnabled(false);
}

void MainWindow::setupCalendar()
{
    m_daysList = new QListWidget(this);
    m_daysList->setFlow(QListWidget::LeftToRight);
    m_daysList->setWrapping(false);
    m_daysList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_daysList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_daysList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_daysList->setFixedHeight(60);

    // Обновленный стиль
    m_daysList->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #e0e0e0;"
        "   background: white;"
        "   outline: 0;"
        "}"
        "QListWidget::item {"
        "   border: 1px solid transparent;"
        "   padding: 2px;"
        "   margin: 1px;"
        "   text-align: center;"
        "   border-radius: 5px;"
        "}"
        "QListWidget::item:hover {"
        "   background: #f5f5f5;"
        "   border: 1px solid #e0e0e0;"
        "}"
        "QListWidget::item:selected {"
        "   background: #4CAF50;"
        "   color: white;"
        "   font-weight: bold;"
        "   border: none;"
        "}"
        "QListWidget::item[background=\"today\"] {"
        "   background: #E8F5E9;"
        "   color: #2E7D32;"
        "   border: 1px solid #A5D6A7;"
        "}"
    );

    connect(m_daysList, &QListWidget::itemClicked, this, &MainWindow::daySelected);
}

void MainWindow::updateDays()
{
    if (!m_daysList) return;

    m_daysList->blockSignals(true);

    QStringList russianShortDays = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    QDate weekStart = m_currentDate.addDays(-(m_currentDate.dayOfWeek() - 1));

    int itemWidth = qMax(50, m_daysList->viewport()->width() / 7 - 2);
    int currentRow = m_daysList->currentRow();

    m_daysList->clear();

    for (int i = 0; i < 7; ++i) {
        QDate date = weekStart.addDays(i);
        QString dayName = russianShortDays[i];

        QListWidgetItem *item = new QListWidgetItem(
            QString("%1\n%2").arg(dayName).arg(date.day()));

        item->setData(Qt::UserRole, date);
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(itemWidth, 50));

        if (date == QDate::currentDate()) {
            item->setData(Qt::UserRole + 1, "today");
        }

        m_daysList->addItem(item);
    }

    if (currentRow >= 0 && currentRow < m_daysList->count()) {
        m_daysList->setCurrentRow(currentRow);
    } else if (m_daysList->count() > 0) {
        // Выделяем текущую дату
        for (int i = 0; i < m_daysList->count(); ++i) {
            if (m_daysList->item(i)->data(Qt::UserRole).toDate() == m_currentDate) {
                m_daysList->setCurrentRow(i);
                break;
            }
        }
    }

    m_daysList->blockSignals(false);
}

void MainWindow::daySelected(QListWidgetItem* item)
{
    m_currentDate = item->data(Qt::UserRole).toDate();
    updateDays();
    updateWorkoutsDisplay();
}

void MainWindow::prevWeek()
{
    m_currentDate = m_currentDate.addDays(-7);
    updateDays();
}

void MainWindow::nextWeek()
{
    m_currentDate = m_currentDate.addDays(7);
    updateDays();
}

void MainWindow::addWorkout()
{
    WorkoutDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        WorkoutData workout;
        workout.type = dialog.getWorkoutType();
        if (workout.type == "Другое") {
            workout.type = dialog.getCustomType();
        }

        workout.duration = dialog.getDuration();
        workout.sets = dialog.getSets();
        workout.reps = dialog.getReps();
        workout.calories = dialog.getCalories();
        workout.notes = dialog.getNotes();
        workout.date = m_currentDate;

        if (!database->openDatabase()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть базу данных");
            return;
        }

        if (database->addWorkout(workout)) {
            workouts.append(workout);
            updateWorkoutsDisplay();
            QMessageBox::information(this, "Успех", "Тренировка добавлена!");
        } else {
            QMessageBox::critical(this, "Ошибка",
                QString("Не удалось добавить тренировку в базу данных.\nОшибка: %1")
                .arg(database->lastError()));
        }
    }
}

void MainWindow::showStats()
{
    StatsDialog statsDialog(workouts, this);
    statsDialog.exec();
}

void MainWindow::toggleWorkoutDetails()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QWidget *detailsWidget = button->property("detailsWidget").value<QWidget*>();
    if (!detailsWidget) return;

    detailsWidget->setVisible(!detailsWidget->isVisible());
    button->setText(detailsWidget->isVisible() ? "▲" : "▼");
}

void MainWindow::updateWorkoutsDisplay()
{
    // Очищаем текущий layout
    QLayoutItem *item;
    while ((item = workoutsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Фильтруем тренировки по текущей дате
    QVector<WorkoutData> todayWorkouts;
    for (const WorkoutData &workout : workouts) {
        if (workout.date == m_currentDate) {
            todayWorkouts.append(workout);
        }
    }

    if (todayWorkouts.isEmpty()) {
        QLabel *noWorkoutsLabel = new QLabel("Нет тренировок на выбранную дату");
        noWorkoutsLabel->setAlignment(Qt::AlignCenter);
        noWorkoutsLabel->setStyleSheet("font-size: 14px; color: #666; margin-top: 20px;");
        workoutsLayout->addWidget(noWorkoutsLabel);
        return;
    }

    for (const WorkoutData &workout : todayWorkouts) {
        QGroupBox *workoutGroup = new QGroupBox();
        workoutGroup->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(workoutGroup, &QGroupBox::customContextMenuRequested,
                this, &MainWindow::showWorkoutContextMenu);

        QVBoxLayout *groupLayout = new QVBoxLayout(workoutGroup);
        groupLayout->setSpacing(0);

        // Заголовок аккордеона
        QWidget *headerWidget = new QWidget();
        QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setContentsMargins(10, 5, 10, 5);

        QLabel *titleLabel = new QLabel(workout.type);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

        QLabel *summaryLabel = new QLabel(QString("%1 мин · %2 подх.").arg(workout.duration).arg(workout.sets));
        summaryLabel->setStyleSheet("color: #666;");

        QPushButton *toggleButton = new QPushButton("▼");
        toggleButton->setFixedSize(24, 24);
        toggleButton->setStyleSheet(
            "QPushButton {"
            "   border: none;"
            "   background: transparent;"
            "   font-size: 12px;"
            "}"
        );

        headerLayout->addWidget(titleLabel);
        headerLayout->addWidget(summaryLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(toggleButton);
        headerWidget->setLayout(headerLayout);

        // Создаем таблицу с деталями
        QTableWidget *detailsTable = new QTableWidget();
        detailsTable->setColumnCount(2);
        detailsTable->setRowCount(4);
        detailsTable->verticalHeader()->hide();
        detailsTable->horizontalHeader()->hide();
        detailsTable->setShowGrid(false);
        detailsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        detailsTable->setSelectionMode(QAbstractItemView::NoSelection);

        detailsTable->setStyleSheet(
            "QTableWidget {"
            "   background-color: #f0f0f0;"
            "   border: 1px solid #c0c0c0;"
            "   border-radius: 5px;"
            "   margin: 5px;"
            "   gridline-color: transparent;"
            "}"
            "QTableWidget QTableCornerButton::section {"
            "   background: #e0e0e0;"
            "   border: 1px solid #c0c0c0;"
            "}"
            "QTableWidget::item {"
            "   padding: 5px 10px;"
            "   border: none;"
            "   background: transparent;"
            "}"
            "QTableWidget::item:selected {"
            "   background: #d8d8d8;"
            "}"
        );


        QPalette palette = detailsTable->palette();
        palette.setColor(QPalette::Base, QColor(240, 240, 240));
        palette.setColor(QPalette::Window, QColor(240, 240, 240));
        detailsTable->setPalette(palette);


        detailsTable->setAutoFillBackground(true);

        // Заполнение таблицы данными
        addTableRow(detailsTable, 0, "Длительность:", QString("%1 мин").arg(workout.duration));
        addTableRow(detailsTable, 1, "Подходы:", QString::number(workout.sets));
        addTableRow(detailsTable, 2, "Повторения:", QString::number(workout.reps));
        addTableRow(detailsTable, 3, "Ккал:", QString::number(workout.calories));

        // Настройка столбцов
        detailsTable->setColumnWidth(0, 120);
        detailsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        detailsTable->setFixedHeight(detailsTable->rowHeight(0) * detailsTable->rowCount() + 12);

        detailsTable->setVisible(false);

        groupLayout->addWidget(headerWidget);
        groupLayout->addWidget(detailsTable);

        // Настройка кнопки аккордеона
        toggleButton->setProperty("detailsWidget", QVariant::fromValue(detailsTable));
        connect(toggleButton, &QPushButton::clicked, this, [this, toggleButton]() {
            QWidget *detailsWidget = toggleButton->property("detailsWidget").value<QWidget*>();
            if (!detailsWidget) return;

            bool isVisible = detailsWidget->isVisible();
            toggleButton->setText(isVisible ? "▼" : "▲");
            detailsWidget->setVisible(!isVisible);
        });

        workoutsLayout->addWidget(workoutGroup);
    }
}

void MainWindow::addTableRow(QTableWidget *table, int row, const QString &label, const QString &value)
{
    QTableWidgetItem *labelItem = new QTableWidgetItem(label);
    labelItem->setFlags(labelItem->flags() ^ Qt::ItemIsEditable);
    labelItem->setForeground(QColor("#505050"));

    QTableWidgetItem *valueItem = new QTableWidgetItem(value);
    valueItem->setFlags(valueItem->flags() ^ Qt::ItemIsEditable);
    valueItem->setForeground(Qt::black);

    table->setItem(row, 0, labelItem);
    table->setItem(row, 1, valueItem);
}

void MainWindow::showWorkoutContextMenu(const QPoint &pos)
{
    QGroupBox *workoutBox = qobject_cast<QGroupBox*>(sender());
    if (!workoutBox) return;

    contextMenuWorkout = workoutBox;

    QMenu contextMenu(this);
    QAction *editAction = contextMenu.addAction("Редактировать");
    QAction *deleteAction = contextMenu.addAction("Удалить");

    connect(editAction, &QAction::triggered, this, &MainWindow::editWorkout);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteWorkout);

    contextMenu.exec(workoutBox->mapToGlobal(pos));
}

void MainWindow::deleteWorkout()
{
    if (!contextMenuWorkout) return;

    int index = findWorkoutIndex(contextMenuWorkout);
    if (index >= 0) {
        if (database->deleteWorkout(workouts[index].id)) {
            workouts.removeAt(index);
            updateWorkoutsDisplay();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить тренировку из базы данных");
        }
    }
}

void MainWindow::editWorkout()
{
    if (!contextMenuWorkout) return;

    int index = findWorkoutIndex(contextMenuWorkout);
    if (index < 0) return;

    WorkoutDialog dialog(this, true);
    dialog.setWorkoutData(workouts[index]);

    if (dialog.exec() == QDialog::Accepted) {
        workouts[index].type = dialog.getWorkoutType();
        if (workouts[index].type == "Другое") {
            workouts[index].type = dialog.getCustomType();
        }
        workouts[index].duration = dialog.getDuration();
        workouts[index].sets = dialog.getSets();
        workouts[index].reps = dialog.getReps();
        workouts[index].calories = dialog.getCalories();
        workouts[index].notes = dialog.getNotes();

        if (database->updateWorkout(workouts[index])) {
            updateWorkoutsDisplay();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось обновить тренировку в базе данных");
        }
    }
}

int MainWindow::findWorkoutIndex(QGroupBox* workoutBox)
{
    for (int i = 0; i < workoutsLayout->count(); ++i) {
        if (workoutsLayout->itemAt(i)->widget() == workoutBox) {
            QString workoutType = workoutBox->findChild<QLabel*>()->text();
            QDate workoutDate = m_currentDate;

            for (int j = 0; j < workouts.size(); ++j) {
                if (workouts[j].type == workoutType && workouts[j].date == workoutDate) {
                    return j;
                }
            }
        }
    }
    return -1;
}

void MainWindow::showCalendarDialog()
{
    QDialog calendarDialog(this);
    calendarDialog.setWindowTitle("Выберите дату");
    calendarDialog.setWindowFlags(calendarDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    calendarDialog.resize(400, 350);
    calendarDialog.setStyleSheet(
        "QDialog {"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(&calendarDialog);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Календарь
    QCalendarWidget *calendar = new QCalendarWidget(&calendarDialog);
    calendar->setSelectedDate(m_currentDate);
    calendar->setStyleSheet(
        "QCalendarWidget {"
        "   background-color: white;"
        "   border: none;"
        "}"
        "QCalendarWidget QWidget {"
        "   background-color: white;"
        "   alternate-background-color: white;"
        "}"
        "QCalendarWidget QToolButton {"
        "   height: 30px;"
        "   font-size: 14px;"
        "   color: #333333;"
        "   background-color: white;"
        "   border: none;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "   background-color: #f5f5f5;"
        "   border-radius: 4px;"
        "}"
        "QCalendarWidget QMenu {"
        "   width: 150px;"
        "   left: 20px;"
        "   background-color: white;"
        "   border: 1px solid #e0e0e0;"
        "}"
        "QCalendarWidget QSpinBox {"
        "   padding: 2px;"
        "   background-color: white;"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 4px;"
        "   min-width: 80px;"
        "   selection-background-color: #4CAF50;"
        "   selection-color: white;"
        "}"
    );


    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // Настройка заголовка (месяц и год)
    QTextCharFormat headerFormat;
    headerFormat.setFont(QFont("Arial", 12, QFont::Normal));
    headerFormat.setBackground(Qt::white);
    headerFormat.setForeground(Qt::black);
    calendar->setHeaderTextFormat(headerFormat);

    QTextCharFormat weekdayFormat;
    weekdayFormat.setFont(QFont("Arial", 10, QFont::Normal));
    weekdayFormat.setForeground(QColor(120, 120, 120, 150));
    weekdayFormat.setBackground(Qt::white);
    for (int i = 1; i <= 7; ++i) {
        calendar->setWeekdayTextFormat(Qt::DayOfWeek(i), weekdayFormat);
    }

    // Настройка форматов для разных типов дат
    QTextCharFormat currentMonthFormat;
    currentMonthFormat.setFont(QFont("Arial", 10, QFont::Normal));
    currentMonthFormat.setForeground(Qt::black);
    currentMonthFormat.setBackground(Qt::white);

    QTextCharFormat otherMonthFormat;
    otherMonthFormat.setFont(QFont("Arial", 10, QFont::Normal));
    otherMonthFormat.setForeground(QColor(180, 180, 180));
    otherMonthFormat.setBackground(Qt::white);

    QTextCharFormat selectedFormat;
    selectedFormat.setBackground(QColor("#4CAF50"));
    selectedFormat.setForeground(Qt::white);
    selectedFormat.setFont(QFont("Arial", 10, QFont::Bold));

    QDate currentDate = QDate::currentDate();
    QDate firstDay = calendar->selectedDate().addDays(-calendar->selectedDate().day() + 1);
    QDate lastDay = firstDay.addMonths(1).addDays(-1);

    calendar->setDateTextFormat(QDate(1900, 1, 1), otherMonthFormat);

    for (QDate date = firstDay; date <= lastDay; date = date.addDays(1)) {
        calendar->setDateTextFormat(date, currentMonthFormat);
    }

    calendar->setDateTextFormat(calendar->selectedDate(), selectedFormat);

    layout->addWidget(calendar, 1);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    QPushButton *cancelButton = new QPushButton("Отменить", &calendarDialog);
    QPushButton *okButton = new QPushButton("OK", &calendarDialog);

    cancelButton->setStyleSheet(
        "QPushButton {"
        "   padding: 6px 12px;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   min-width: 70px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d32f2f;"
        "}"
    );

    okButton->setStyleSheet(
        "QPushButton {"
        "   padding: 6px 12px;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   min-width: 70px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);

    layout->addLayout(buttonLayout);

    // Обработчик изменения месяца
    connect(calendar, &QCalendarWidget::currentPageChanged, [=](int year, int month) {
        QDate firstDay(year, month, 1);
        QDate lastDay = firstDay.addMonths(1).addDays(-1);

        calendar->setDateTextFormat(QDate(1900, 1, 1), otherMonthFormat);

        for (QDate date = firstDay; date <= lastDay; date = date.addDays(1)) {
            calendar->setDateTextFormat(date, currentMonthFormat);
        }
        calendar->setDateTextFormat(calendar->selectedDate(), selectedFormat);
    });

    connect(okButton, &QPushButton::clicked, &calendarDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &calendarDialog, &QDialog::reject);

    if (calendarDialog.exec() == QDialog::Accepted) {
        m_currentDate = calendar->selectedDate();
        currentDateLabel->setText(
            QLocale(QLocale::Russian).monthName(m_currentDate.month()) +
            " " + QString::number(m_currentDate.year())
        );
        updateDays();
        updateWorkoutsDisplay();
    }
}
