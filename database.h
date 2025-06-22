#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// Предварительное объявление структуры WorkoutData
struct WorkoutData;

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool openDatabase();
    void closeDatabase();
    QString lastError() const;

    bool addWorkout(const WorkoutData &workout);
    QVector<WorkoutData> getAllWorkouts();
    bool updateWorkout(const WorkoutData &workout);
    bool deleteWorkout(int id);

private:
    bool checkTables();
    QSqlDatabase db;
};

#endif // DATABASE_H
