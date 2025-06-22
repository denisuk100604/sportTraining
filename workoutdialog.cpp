#include "workoutdialog.h"
#include <QLabel>
#include <QApplication>

WorkoutDialog::WorkoutDialog(QWidget *parent, bool isEditMode)
    : QDialog(parent)
{
    setWindowTitle(isEditMode ? "Редактирование тренировки" : "Добавить тренировку");
    setModal(true);
    resize(400, 300);

    QApplication::setStyle("Fusion");

    setStyleSheet(
        // Базовые стили
        "QDialog {"
        "   background-color: #f5f5f5;"
        "   font-family: Arial;"
        "}"

        "QLabel {"
        "   font-weight: bold;"
        "   color: #333;"
        "}"

        // Стили для элементов ввода
        "QComboBox, QSpinBox, QLineEdit {"
        "   padding: 5px;"
        "   border: 1px solid #ccc;"
        "   border-radius: 3px;"
        "   background: white;"
        "}"

        // Стили для кнопок
        "QPushButton {"
        "   padding: 8px 15px;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "   min-width: 80px;"
        "}"

        "QSpinBox {"
        "   padding-right: 20px;"
        "}"

        "QSpinBox::up-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: top right;"
        "   width: 20px;"
        "   height: 8px;"

        "}"
        "QSpinBox::down-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: bottom right;"
        "   width: 20px;"
        "   height: 8px;"
        "}"


        "QPushButton#saveButton:hover {"
        "   background-color: #45a049;"
        "}"

        "QPushButton#cancelButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: 1px solid #d32f2f;"
        "}"

        "QPushButton#cancelButton:hover {"
        "   background-color: #d32f2f;"
        "}"

        "QComboBox QAbstractItemView {"
        "   background: white;"
        "   color: black;"
        "   selection-background-color: #4CAF50;"
        "   selection-color: white;"
        "}"

        "QComboBox {"
        "   padding: 5px;"
        "   border: 1px solid #ccc;"
        "   border-radius: 3px;"
        "   background: white;"
        "   color: black;"
        "}"
        "QComboBox:hover {"
        "   border: 1px solid #888;"
        "   background: white;"
        "   color: black;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background: white;"
        "   color: black;"
        "   selection-background-color: #4CAF50;"
        "   selection-color: white;"
        "   outline: none;"
        "}"
    );

    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Тип тренировки
    typeCombo = new QComboBox(this);
    typeCombo->addItems({"Кардио", "Силовая", "Йога", "Плавание", "Велоспорт", "Кроссфит", "Другое"});
    layout->addRow("Тип тренировки:", typeCombo);

    // Пользовательский тип
    customTypeEdit = new QLineEdit(this);
    customTypeEdit->setPlaceholderText("Введите тип тренировки");
    customTypeEdit->setVisible(false);

    // Длительность
    durationSpin = new QSpinBox(this);
    durationSpin->setRange(1, 300);
    durationSpin->setSuffix(" мин");
    layout->addRow("Длительность:", durationSpin);

    // Подходы и повторения
    setsSpin = new QSpinBox(this);
    setsSpin->setRange(0, 50);
    layout->addRow("Количество подходов:", setsSpin);

    repsSpin = new QSpinBox(this);
    repsSpin->setRange(0, 100);
    layout->addRow("Повторений в подходе:", repsSpin);

    // Килокалории
    caloriesSpin = new QSpinBox(this);
    caloriesSpin->setRange(0, 2000);
    caloriesSpin->setSuffix(" ккал");
    layout->addRow("Сожжённые Ккал:", caloriesSpin);

    // Заметки
    notesEdit = new QLineEdit(this);
    notesEdit->setPlaceholderText("Дополнительные заметки");
    layout->addRow("Заметки:", notesEdit);

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *cancelButton = new QPushButton("Отмена", this);
    cancelButton->setObjectName("cancelButton");
    QPushButton *saveButton = new QPushButton("Сохранить", this);
    saveButton->setObjectName("saveButton");
    saveButton->setDefault(true);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);

    layout->addRow(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(typeCombo, &QComboBox::currentTextChanged, [this](const QString &text){
        customTypeEdit->setVisible(text == "Другое");
        if (text == "Другое") {
            static_cast<QFormLayout*>(this->layout())->insertRow(1, customTypeEdit);
        } else {
            customTypeEdit->setParent(nullptr);
        }
    });
}

// Реализации методов получения данных
QString WorkoutDialog::getWorkoutType() const {
    return typeCombo->currentText();
}

QString WorkoutDialog::getCustomType() const {
    return customTypeEdit->text();
}

int WorkoutDialog::getDuration() const {
    return durationSpin->value();
}

int WorkoutDialog::getSets() const {
    return setsSpin->value();
}

int WorkoutDialog::getReps() const {
    return repsSpin->value();
}

int WorkoutDialog::getCalories() const {
    return caloriesSpin->value();
}

QString WorkoutDialog::getNotes() const {
    return notesEdit->text();
}

void WorkoutDialog::setWorkoutData(const WorkoutData &workout)
{
    // Устанавливаем значения в форму редактирования
    int typeIndex = typeCombo->findText(workout.type);
    if (typeIndex >= 0) {
        typeCombo->setCurrentIndex(typeIndex);
    } else {
        typeCombo->setCurrentIndex(typeCombo->count() - 1);
        customTypeEdit->setText(workout.type);
        customTypeEdit->setVisible(true);
    }

    durationSpin->setValue(workout.duration);
    setsSpin->setValue(workout.sets);
    repsSpin->setValue(workout.reps);
    caloriesSpin->setValue(workout.calories);
    notesEdit->setText(workout.notes);
}
