#include "database.h"
#include "mainwindow.h"
#include <QDir>
#include <QDebug>

Database::Database(QObject *parent) : QObject(parent)
{
    // Убедимся, что соединение с таким именем не существует
    if(QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("workout_tracker.db");
    }

    qDebug() << "Database path:" << QDir().absoluteFilePath(db.databaseName());

    if (!openDatabase()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return;
    }

    if (!checkTables()) {
        qDebug() << "Table 'workouts' doesn't exist. Creating...";
        QSqlQuery query(db);
        if (!query.exec("CREATE TABLE workouts ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "type TEXT NOT NULL, "
                       "duration INTEGER NOT NULL, "
                       "sets INTEGER, "
                       "reps INTEGER, "
                       "calories INTEGER, "
                       "notes TEXT, "
                       "date TEXT NOT NULL)")) {
            qDebug() << "Failed to create table:" << query.lastError().text();
        }
    }
}

Database::~Database()
{

}

bool Database::openDatabase()
{
    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
        return false;
    }
    return true;
}

void Database::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::addWorkout(const WorkoutData &workout)
{
    if (!db.isOpen()) {
        qDebug() << "Database is not open! Attempting to reopen...";
        if (!openDatabase()) {
            qDebug() << "Failed to reopen database:" << db.lastError().text();
            return false;
        }
    }

    qDebug() << "Attempting to add workout with data:";
    qDebug() << "Type:" << workout.type;
    qDebug() << "Duration:" << workout.duration;
    qDebug() << "Sets:" << workout.sets;
    qDebug() << "Reps:" << workout.reps;
    qDebug() << "Calories:" << workout.calories;
    qDebug() << "Notes:" << workout.notes;
    qDebug() << "Date:" << workout.date.toString("yyyy-MM-dd");

    QSqlQuery query;
    QString sql = "INSERT INTO workouts (type, duration, sets, reps, calories, notes, date) "
                  "VALUES (:type, :duration, :sets, :reps, :calories, :notes, :date)";

    qDebug() << "Preparing SQL:" << sql;

    if (!query.prepare(sql)) {
        qDebug() << "Prepare failed:" << query.lastError().text();
        return false;
    }

    query.bindValue(":type", workout.type);
    query.bindValue(":duration", workout.duration);
    query.bindValue(":sets", workout.sets);
    query.bindValue(":reps", workout.reps);
    query.bindValue(":calories", workout.calories);
    query.bindValue(":notes", workout.notes);
    query.bindValue(":date", workout.date.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Execution failed:" << query.lastError().text();
        qDebug() << "Last query:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        return false;
    }

    qDebug() << "Workout added successfully!";
    return true;
}

QVector<WorkoutData> Database::getAllWorkouts()
{
    QVector<WorkoutData> workouts;
    if (!db.isOpen() && !openDatabase()) {
        return workouts;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM workouts ORDER BY date DESC")) {
        qDebug() << "Query failed:" << query.lastError().text();
        return workouts;
    }

    while (query.next()) {
        WorkoutData workout;
        workout.id = query.value("id").toInt();
        workout.type = query.value("type").toString();
        workout.duration = query.value("duration").toInt();
        workout.sets = query.value("sets").toInt();
        workout.reps = query.value("reps").toInt();
        workout.calories = query.value("calories").toInt();
        workout.notes = query.value("notes").toString();
        workout.date = QDate::fromString(query.value("date").toString(), "yyyy-MM-dd");

        workouts.append(workout);
    }
    return workouts;
}

bool Database::updateWorkout(const WorkoutData &workout)
{
    if (!db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("UPDATE workouts SET type = :type, duration = :duration, sets = :sets, "
                  "reps = :reps, calories = :calories, notes = :notes, date = :date "
                  "WHERE id = :id");
    query.bindValue(":type", workout.type);
    query.bindValue(":duration", workout.duration);
    query.bindValue(":sets", workout.sets);
    query.bindValue(":reps", workout.reps);
    query.bindValue(":calories", workout.calories);
    query.bindValue(":notes", workout.notes);
    query.bindValue(":date", workout.date.toString("yyyy-MM-dd"));
    query.bindValue(":id", workout.id);

    if (!query.exec()) {
        qDebug() << "Update workout error:" << query.lastError();
        return false;
    }
    return true;
}

bool Database::deleteWorkout(int id)
{
    if (!db.isOpen()) return false;

    QSqlQuery query;
    query.prepare("DELETE FROM workouts WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Delete workout error:" << query.lastError();
        return false;
    }
    return true;
}

QString Database::lastError() const
{
    return db.lastError().text();
}

bool Database::checkTables()
{
    if (!db.isOpen()) return false;

    return db.tables().contains("workouts");
}
