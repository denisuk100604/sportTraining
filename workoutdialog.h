#ifndef WORKOUTDIALOG_H
#define WORKOUTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "mainwindow.h"

class WorkoutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WorkoutDialog(QWidget *parent = nullptr, bool isEditMode = false);

    QString getWorkoutType() const;
    QString getCustomType() const;
    int getDuration() const;
    int getSets() const;
    int getReps() const;
    int getCalories() const;
    QString getNotes() const;

    void setWorkoutData(const WorkoutData &workout);

private:
    void setupUi();
    QComboBox *typeCombo;
    QLineEdit *customTypeEdit;
    QSpinBox *durationSpin;
    QSpinBox *setsSpin;
    QSpinBox *repsSpin;
    QSpinBox *caloriesSpin;
    QLineEdit *notesEdit;
};

#endif // WORKOUTDIALOG_H
